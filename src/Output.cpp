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
