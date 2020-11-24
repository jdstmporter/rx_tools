/*
 * Dongle.cpp
 *
 *  Created on: 22 Nov 2020
 *      Author: julianporter
 */

#include "Dongle.hpp"
#include "Demod.hpp"
#include "Output.hpp"
#include "convenience.h"
#include "SDR.hpp"

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


void Dongle::rtlsdr_callback(int16_t *buf,uint32_t len) {
	auto demod_target = SDR::shared()->demod;
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

void Dongle::threadFunction() {

	SoapySDRDevice_activateStream(dev, stream, 0, 0, 0);
	size_t samples_per_buffer = MAXIMUM_BUF_LENGTH/2; //fix for int16 storage
	size_t elemsize = SoapySDR_formatToSize(SOAPY_SDR_CS16);
	int16_t *buf = malloc(samples_per_buffer * elemsize);
	memset(buf, 0, samples_per_buffer * elemsize);
	if (!buf) {
		perror("malloc");
		exit(1);
	}

	auto output=SDR::shared()->output;
	auto demod_target = SDR::shared()->demod;
	suppress_stdout_stop(output->tmp_stdout);
	if (output->wav_format) {
		output->generateHeader(demod_target->demodKind()==Demodulator::Kind::RAW);
	}

	do
	{
		void *buffs[] = {buf};
		int flags = 0;
		long long timeNs = 0;
		long timeoutNs = 1000000;

		int r = SoapySDRDevice_readStream(dev, stream, buffs, samples_per_buffer, &flags, &timeNs, timeoutNs);
		//fprintf(stderr, "ret=%d\n", r);

		if (r >= 0) {
			// r is number of elements read, elements=complex pairs, so buffer length in bytes is twice
			rtlsdr_callback(buf, r * 2);
		} else {
			if (r == SOAPY_SDR_OVERFLOW) {
				fprintf(stderr, "O");
				fflush(stderr);
				continue;
			}
			fprintf(stderr, "readStream read failed: %d\n", r);
			break;
		}
	} while(1);
	fprintf(stderr, "dongle_thread_fn terminated\n");

}
