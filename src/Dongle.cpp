/*
 * Dongle.cpp
 *
 *  Created on: 22 Nov 2020
 *      Author: julianporter
 */

#include "Dongle.hpp"
#include "Demod.hpp"

void Dongle::rotate16_90(uint32_t len)
/* 90 rotation is 1+0j, 0+1j, -1+0j, 0-1j
   or [0, 1, -3, 2, -4, -5, 7, -6] */
{
	uint32_t i;
	int16_t tmp;
	for (i=0; i<len; i+=8) {
		tmp = - buf16[i+3];
		buf16[i+3] = buf16[i+2];
		buf16[i+2] = tmp;

		buf16[i+4] = - buf16[i+4];
		buf16[i+5] = - buf16[i+5];

		tmp = - buf16[i+6];
		buf16[i+6] = buf16[i+7];
		buf16[i+7] = tmp;
	}
}


void Dongle::process(int16_t *buf,uint32_t len) {
	if (mute) {
		for (auto i=0; i<mute; i++) buf[i] = 0;
		mute = 0;
	}
	for (auto i=0; i<(int)len; i++) {
		buf16[i] = ( (int16_t)buf[i] / 32767.0 * 128.0 + 0.4);
		// TODO: remove downconversion from 16-bit to 8-bit
	}

	/* 2nd: do DC filtering BEFORE up-mixing */
	if (demod_target->dc_block_raw) {
		demod_target->dc_block_raw_filter(buf16, (int)len);
	}
	/* 3rd: up-mixing */
	if (!offset_tuning) {
		rotate16_90(len);
		/* rotate_90(buf, len); */
	}
	pthread_rwlock_wrlock(&demod_target->rw);
	memcpy(demod_target->lowpassed, buf16, 2*len);
	demod_target->lp_len = len;
	pthread_rwlock_unlock(&demod_target->rw);
	safe_cond_signal(&demod_target->ready, &demod_target->ready_m);

}
