/*
 * Output.hpp
 *
 *  Created on: 23 Nov 2020
 *      Author: julianporter
 */

#ifndef SRC_OUTPUT_HPP_
#define SRC_OUTPUT_HPP_

#include "base.hpp"

struct output_state
{
	int	  exit_flag;
	pthread_t thread;
	FILE	 *file;
	char	 *filename;
	int16_t  result[MAXIMUM_BUF_LENGTH];
	int	  result_len;
	int	  rate;
	int	  wav_format;
	pthread_rwlock_t rw;
	pthread_cond_t ready;
	pthread_mutex_t ready_m;
};

class Output {
	int	  exit_flag = 0;
	pthread_t thread = 0;
	FILE	 *file = nullptr;
	char	 *filename = nullptr;
	int16_t  result[MAXIMUM_BUF_LENGTH];
	int	  result_len = 0;
	int	  rate = 0;
	bool	  wav_format = false;
	pthread_rwlock_t rw;
	pthread_cond_t ready;
	pthread_mutex_t ready_m;

	Output();
	virtual ~Output();
};

#endif /* SRC_OUTPUT_HPP_ */
