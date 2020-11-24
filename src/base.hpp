/*
 * base.hpp
 *
 *  Created on: 22 Nov 2020
 *      Author: julianporter
 */

#ifndef SRC_BASE_HPP_
#define SRC_BASE_HPP_

#include <errno.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <pthread.h>
#include <memory>

#include <SoapySDR/Device.h>
#include <SoapySDR/Formats.h>
#include "ExitFlag.hpp"

#define DEFAULT_SAMPLE_RATE		24000
#define DEFAULT_BUF_LENGTH		(1 * 16384)
#define MAXIMUM_OVERSAMPLE		16
#define MAXIMUM_BUF_LENGTH		(MAXIMUM_OVERSAMPLE * DEFAULT_BUF_LENGTH)
#define BUFFER_DUMP				4096

#define FREQUENCIES_LIMIT		1000

#define safe_cond_signal(n, m) pthread_mutex_lock(m); pthread_cond_signal(n); pthread_mutex_unlock(m)
#define safe_cond_wait(n, m) pthread_mutex_lock(m); pthread_cond_wait(n, m); pthread_mutex_unlock(m)




#endif /* SRC_BASE_HPP_ */
