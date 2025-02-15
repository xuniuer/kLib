/***********************************************************************************
 *                                                                                 *
 *   kLib - C++ development tools for ARM Cortex-M devices                         *
 *                                                                                 *
 *     Copyright (c) 2018, project author Pawel Zalewski                           *
 *     All rights reserved.                                                        *
 *                                                                                 *
 ***********************************************************************************
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 *   1. Redistributions of source code must retain the above copyright notice,
 *      this list of conditions and the following disclaimer.
 *   2. Redistributions  in  binary  form  must  reproduce the above copyright
 *      notice,  this  list  of conditions and the following disclaimer in the
 *      documentation  and/or  other materials provided with the distribution.
 *   3. Neither  the  name  of  the  copyright  holder  nor  the  names of its
 *      contributors  may  be used to endorse or promote products derived from
 *      this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,  BUT NOT LIMITED  TO, THE
 *   IMPLIED WARRANTIES OF MERCHANTABILITY  AND FITNESS FOR A PARTICULAR PURPOSE
 *   ARE DISCLAIMED.  IN NO EVENT SHALL  THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *   LIABLE  FOR  ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR
 *   CONSEQUENTIAL  DAMAGES  (INCLUDING,  BUT  NOT  LIMITED  TO,  PROCUREMENT OF
 *   SUBSTITUTE  GOODS  OR SERVICES;  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *   INTERRUPTION) HOWEVER  CAUSED  AND  ON  ANY THEORY OF LIABILITY, WHETHER IN
 *   CONTRACT,  STRICT  LIABILITY,  OR  TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *   ARISING  IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *   POSSIBILITY OF SUCH DAMAGE.
 *
 */



#include "kSDCard.h"

kSDCard::kSDCard(void)
{
	this->prvVersion = 0;
	this->prvStatus = kSD::NoInit;
}
unsigned char kSDCard::getStatus(void)
{
	return this->prvStatus;
}
void kSDCard::run(unsigned int sck_freq)
{
	//this->hardware = kSPI::Master->MSB_First.SCK_IdleLow.DataCapture_1Edge;
	kSPI::run(sck_freq);
}
unsigned char kSDCard::init(void)
{
    unsigned char n, ty, ocr[4];
    unsigned int delay_counter;
    this->deselect();
    for(n=0;n<10;n++) this->write(0xFF);
    this->select();

    ty = 0;
    if (this->writeCMD(kSD::CMD0, 0) == 1) {            /* Enter Idle state */
        delay_counter = kSystem.millis() - 1000;        /* Initialization timeout of 1000 msec */
        if (this->writeCMD(kSD::CMD8, 0x1AA) == 1) {    /* SDC Ver2+ */
            for (n = 0; n < 4; n++) ocr[n] = this->read();
            if (ocr[2] == 0x01 && ocr[3] == 0xAA) {    /* The card can work at vdd range of 2.7-3.6V */
                do {
                    if (this->writeCMD(kSD::CMD55, 0) <= 1 && this->writeCMD(kSD::CMD41, 1UL << 30) == 0)    break;    /* ACMD41 with HCS bit */
                } while (!kSystem.isTimePassed(&delay_counter));
                if ((!kSystem.isTimePassed(&delay_counter)) && this->writeCMD(kSD::CMD58, 0) == 0) {    /* Check CCS bit */
                    for (n = 0; n < 4; n++) ocr[n] = this->read();
                    ty = (ocr[0] & 0x40) ? 6 : 2;
                }
            }
        } else {                            /* SDC Ver1 or MMC */
            ty = (this->writeCMD(kSD::CMD55, 0) <= 1 && this->writeCMD(kSD::CMD41, 0) <= 1) ? 2 : 1;    /* SDC : MMC */
            do {
                if (ty == 2) {
                    if (this->writeCMD(kSD::CMD55, 0) <= 1 && this->writeCMD(kSD::CMD41, 0) == 0) break;    /* ACMD41 */
                } else {
                    if (this->writeCMD(kSD::CMD1, 0) == 0) break;                                /* CMD1 */
                }
            } while (!kSystem.isTimePassed(&delay_counter));
            if (kSystem.isTimePassed(&delay_counter) || this->writeCMD(kSD::CMD16, 512) != 0)    /* Select R/W block length */
                ty = 0;
        }
    }
    this->prvVersion = ty;
    this->deselect();
    this->read();           /* Idle (Release DO) */

    if(ty) *((unsigned char*)&this->prvStatus) &= (~((unsigned char)kSD::NoInit));        /* Clear STA_NOINIT */

    return (unsigned char)this->prvStatus;
}

unsigned char kSDCard::waitReady(void)
{
    unsigned char res;
    unsigned int delay_counter = kSystem.millis() - 500;

    this->read();
    do
        res = this->read();
    while ((res != 0xFF) && (!kSystem.isTimePassed(&delay_counter)));

    return res;
}
unsigned char kSDCard::writeCMD(unsigned char cmd,unsigned int arg)
{
    unsigned char n, res;

    this->waitReady();

    // Send command packet
    this->write(cmd);     		                   /* Command */
    this->write((unsigned char)(arg >> 24));        /* Argument[31..24] */
    this->write((unsigned char)(arg >> 16));        /* Argument[23..16] */
    this->write((unsigned char)(arg >> 8));            /* Argument[15..8] */
    this->write((unsigned char)arg);                /* Argument[7..0] */

    // send CRC
    n = 0;
    if (cmd == kSD::CMD0) n = 0x95;            /* CRC for CMD0(0) */
    if (cmd == kSD::CMD8) n = 0x87;            /* CRC for CMD8(0x1AA) */
    this->write(n);

    /* Receive command response */
    if (cmd == kSD::CMD12) this->read();        /* Skip a stuff byte when stop reading */
    n = 10;                                /* Wait for a valid response in timeout of 10 attempts */
    do
        res = this->read();
    while ((res & 0x80) && --n);


    return res;            /* Return with the response value */
}
unsigned char kSDCard::readSector(unsigned char * buff, unsigned long sector, unsigned int count)
{
    if (!count) return kSD::PARERR;
    if (this->prvStatus & ((unsigned char)kSD::NoInit)) return kSD::NOTRDY;

    if (!(this->prvVersion & 4)) sector *= 512;    /* Convert to byte address if needed */

    this->select();            /* CS = L */

    if (count == 1) {    /* Single block read */
        if ((this->writeCMD(kSD::CMD17, sector) == 0)    /* READ_SINGLE_BLOCK */
            && rcvr_datablock(buff,512))
            count = 0;
    }
    else {                /* Multiple block read */
        if (this->writeCMD(kSD::CMD18, sector) == 0) {    /* READ_MULTIPLE_BLOCK */
            do {
                if (!rcvr_datablock(buff,512)) break;
                buff += 512;
            } while (--count);
            this->writeCMD(kSD::CMD12, 0);                /* STOP_TRANSMISSION */
        }
    }

    this->deselect();           /* CS = H */
    this->read();            	/* Idle (Release DO) */

    return count ? kSD::ERROR : kSD::OK;
}
unsigned char kSDCard::writeSector(const unsigned char* buff, unsigned long sector, unsigned int count)
{
    if (this->prvStatus & ((unsigned char)kSD::NoInit)) return kSD::NOTRDY;
    if (this->prvStatus & ((unsigned char)kSD::WriteProtect)) return kSD::WRPRT;

    if (!(this->prvVersion & 4)) sector *= 512;    /* Convert to byte address if needed */

    this->select();            /* CS = L */

    if (count == 1) {    /* Single block write */
        if ((this->writeCMD(kSD::CMD24, sector) == 0)    /* WRITE_BLOCK */
            && this-> xmit_datablock((unsigned char *)buff, 0xFE))
            count = 0;
    }
    else {                /* Multiple block write */
        if (this->prvVersion & 2) {
            this->writeCMD(kSD::CMD55, 0); this->writeCMD(kSD::CMD23, count);    /* ACMD23 */
        }
        if (this->writeCMD(kSD::CMD25, sector) == 0) {    /* WRITE_MULTIPLE_BLOCK */
            do {
                if (!this->xmit_datablock((unsigned char *)buff, 0xFC)) break;
                buff += 512;
            } while (--count);
            if (!xmit_datablock(0, 0xFD))    /* STOP_TRAN token */
                count = 1;
        }
    }

    this->deselect();            /* CS = H */
    this->read();            /* Idle (Release DO) */

    return count ? kSD::ERROR : kSD::OK;
}

bool kSDCard::xmit_datablock (unsigned char * buff, unsigned char token)
{
	unsigned char resp;
	unsigned int i = 0;

    if (this->waitReady() != 0xFF) return false;

    this->write(token);                    /* Xmit data token */
    if (token != 0xFD)
	{    /* Is data token */
    	this->write(buff,512);
    	this->read();
    	this->read();

    	while (i <= 64)
		{
        	resp = this->read();                /* Reveive data response */
        	if ((resp & 0x1F) == 0x05) break;       /* If not accepted, return with error */
			i++;
    	}
		while (this->read() == 0);
	}
	if ((resp & 0x1F) == 0x05) return true;
	return false;
}
bool kSDCard::rcvr_datablock (unsigned char * buff, unsigned int btr)
{
    unsigned char token;
    unsigned int delay_counter = kSystem.millis() - 100;


    do {                            /* Wait for data packet in timeout of 100ms */
        token = this->read();
    } while ((token == 0xFF) && (!kSystem.isTimePassed(&delay_counter)));
    if(token != 0xFE) return false;    /* If not valid data token, retutn with error */

    this->read(btr,buff);

    this->read();                        /* Discard CRC */
    this->read();

    return true;                    /* Return with success */
}

kSD::result_t kSDCard::ioctl(unsigned char cmd, void* buff)
{
	kSD::result_t res;
    uint8_t n, csd[16];
    uint16_t csize;

    if (this->prvStatus & kSD::NoInit) return kSD::NOTRDY;

	res = kSD::ERROR;

	this->select();        /* CS = L */



	switch (cmd) {
        case kSD::GET_SECTOR_COUNT:    /* Get number of sectors on the disk (DWORD) */
            if ((this->writeCMD(kSD::CMD9, 0) == 0) && this->rcvr_datablock(csd, 16)) {
                if ((csd[0] >> 6) == 1) {    /* SDC ver 2.00 */
                    csize = csd[9] + ((uint16_t)csd[8] << 8) + 1;
                    *(uint32_t*)buff = (uint32_t)csize << 10;
                } else {                    /* MMC or SDC ver 1.XX */
                    n = (csd[5] & 15) + ((csd[10] & 128) >> 7) + ((csd[9] & 3) << 1) + 2;
                    csize = (csd[8] >> 6) + ((uint16_t)csd[7] << 2) + ((uint16_t)(csd[6] & 3) << 10) + 1;
                    *(uint32_t*)buff = (uint32_t)csize << (n - 9);
                }
                res = kSD::OK;
            }
            break;

        case kSD::GET_SECTOR_SIZE :    /* Get sectors on the disk (WORD) */
            *(uint16_t*)buff = 512;
            res = kSD::OK;
            break;

        case kSD::CTRL_SYNC :    /* Make sure that data has been written */
            if (this->waitReady() == 0xFF)
                res = kSD::OK;
            break;
        case kSD::GET_BLOCK_SIZE:
        	*(uint16_t*)buff = 1;
			return kSD::OK;
		case kSD::CTRL_TRIM:
			return kSD::OK;
		break;
        default:
            res = kSD::ERROR;
	}

	this->deselect();            /* CS = H */
	this->kSPI::read();    /* Idle (Release DO) */

    return res;
}
