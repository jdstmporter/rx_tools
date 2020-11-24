/*
 * Controller.hpp
 *
 *  Created on: 23 Nov 2020
 *      Author: julianporter
 */

#ifndef SRC_CONTROLLER_HPP_
#define SRC_CONTROLLER_HPP_

#include "base.hpp"
#include "Dongle.hpp"

struct controller_state
{
	int	  exit_flag;
	pthread_t thread;
	uint32_t freqs[FREQUENCIES_LIMIT];
	int	  freq_len;
	int	  freq_now;
	int	  edge;
	int	  wb_mode;
	pthread_cond_t hop;
	pthread_mutex_t hop_m;
};

struct Controller {
	static int verbosity = 0;

	int	  exit_flag = 0;
	pthread_t thread = 0;
	uint32_t freqs[FREQUENCIES_LIMIT];
	int	  freq_len = 0;
	int	  freq_now = 0;
	int	  edge = 0;
	int	  wb_mode = 0;
	pthread_cond_t hop;
	pthread_mutex_t hop_m;

	int ACTUAL_BUF_LENGTH();
	Controller();
	virtual ~Controller();

	void frequency_range( char *arg);
	void sanity_checks();
	void optimal_settings(int freq, int rate);
	void threadFunction();
};

#endif /* SRC_CONTROLLER_HPP_ */
