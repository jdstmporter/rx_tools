/*
 * Output.cpp
 *
 *  Created on: 23 Nov 2020
 *      Author: julianporter
 */

#include "Output.hpp"

Output::Output() {
	pthread_rwlock_init(&rw, NULL);
	pthread_cond_init(&ready, NULL);
	pthread_mutex_init(&ready_m, NULL);
}

Output::~Output() {
	pthread_rwlock_destroy(&rw);
	pthread_cond_destroy(&ready);
	pthread_mutex_destroy(&ready_m);
}

void Output::threadFunction() {
		while (!doExit) {
			// use timedwait and pad out under runs
			safe_cond_wait(&ready, &ready_m);
			pthread_rwlock_rdlock(&rw);
			fwrite(result, 2, result_len, file);
			pthread_rwlock_unlock(&rw);
		}
}

int Output::generateHeader(const bool isRaw) {
	int i, s_rate, b_rate;
	char *channels = "\1\0";
	char *align = "\2\0";
	uint8_t samp_rate[4] = {0, 0, 0, 0};
	uint8_t byte_rate[4] = {0, 0, 0, 0};
	s_rate =rate;
	b_rate =rate * 2;
	if (isRaw) {
		channels = "\2\0";
		align = "\4\0";
		b_rate *= 2;
	}
	for (i=0; i<4; i++) {
		samp_rate[i] = (uint8_t)((s_rate >> (8*i)) & 0xFF);
		byte_rate[i] = (uint8_t)((b_rate >> (8*i)) & 0xFF);
	}
	fwrite("RIFF",     1, 4, file);
	fwrite("\xFF\xFF\xFF\xFF", 1, 4, file);  /* size */
	fwrite("WAVE",     1, 4, file);
	fwrite("fmt ",     1, 4, file);
	fwrite("\x10\0\0\0", 1, 4, file);  /* size */
	fwrite("\1\0",     1, 2, file);  /* pcm */
	fwrite(channels,   1, 2, file);
	fwrite(samp_rate,  1, 4, file);
	fwrite(byte_rate,  1, 4, file);
	fwrite(align, 1, 2, file);
	fwrite("\x10\0",     1, 2, file);  /* bits per channel */
	fwrite("data",     1, 4, file);
	fwrite("\xFF\xFF\xFF\xFF", 1, 4, file);  /* size */
	return 0;
}
