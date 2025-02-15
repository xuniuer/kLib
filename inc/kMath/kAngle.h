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



#ifndef __kAngle_H
#define __kAngle_H


	class kAngle;
	class kAngle_minus_pi_to_pi;

	class kAngle_0_to_2pi
	{
		friend class kAngle;
		friend class kAngle_minus_pi_to_pi;

		protected:

			float value;

		public:


			kAngle_0_to_2pi(const kAngle_minus_pi_to_pi & other);
			kAngle_0_to_2pi(const float & f);

			operator float();

			void operator = (float & val);
			void operator += (float & val);
			void operator -= (float & val);
			void operator *= (float & val);
			void operator /= (float & val);

			void operator = (kAngle_0_to_2pi & ang);
			void operator += (kAngle_0_to_2pi & ang);
			void operator -= (kAngle_0_to_2pi & ang);
			void operator *= (kAngle_0_to_2pi & ang);
			void operator /= (kAngle_0_to_2pi & ang);

			float operator +(kAngle_0_to_2pi & ang);
			float operator -(kAngle_0_to_2pi & ang);
			float operator *(kAngle_0_to_2pi & ang);
			float operator /(kAngle_0_to_2pi & ang);

			bool operator == (kAngle_0_to_2pi & ang);
			bool operator > (kAngle_0_to_2pi & ang);
			bool operator < (kAngle_0_to_2pi & ang);

			void operator = (kAngle_minus_pi_to_pi & ang);
	};


	class kAngle_minus_pi_to_pi
	{
		friend class kAngle;
		friend class kAngle_0_to_2pi;

		protected:

			float value;

		public:


			kAngle_minus_pi_to_pi(const kAngle_0_to_2pi & other);
			kAngle_minus_pi_to_pi(const float & f);

			operator float();


			void operator = (float & val);
			void operator += (float & val);
			void operator -= (float & val);
			void operator *= (float & val);
			void operator /= (float & val);

			void operator = (kAngle_minus_pi_to_pi & ang);
			void operator += (kAngle_minus_pi_to_pi & ang);
			void operator -= (kAngle_minus_pi_to_pi & ang);
			void operator *= (kAngle_minus_pi_to_pi & ang);
			void operator /= (kAngle_minus_pi_to_pi & ang);

			float operator +(kAngle_minus_pi_to_pi & ang);
			float operator -(kAngle_minus_pi_to_pi & ang);
			float operator *(kAngle_minus_pi_to_pi & ang);
			float operator /(kAngle_minus_pi_to_pi & ang);

			bool operator == (kAngle_minus_pi_to_pi & ang);
			bool operator > (kAngle_minus_pi_to_pi & ang);
			bool operator < (kAngle_minus_pi_to_pi & ang);

			void operator = (kAngle_0_to_2pi & ang);
	};




	class kAngle
	{

		public:

		static float setInBounds_0_to_2pi(float angle_in_radians);
		static float setInBounds_minus_pi_to_pi(float angle_in_radians);

		static float getAngleError(kAngle_0_to_2pi & reference_signal,kAngle_0_to_2pi & signal);
		static float getAngleError(kAngle_minus_pi_to_pi & reference_signal,kAngle_minus_pi_to_pi & signal);
	};

#endif
