/*
 * Controller.cpp
 *
 *  Created on: 23 Nov 2020
 *      Author: julianporter
 */

#include "Controller.hpp"

Controller::Controller() {
	freqs[0] = 100000000;
	pthread_cond_init(&hop, NULL);
	pthread_mutex_init(&hop_m, NULL);
}

Controller::~Controller() {
	pthread_cond_destroy(&hop);
	pthread_mutex_destroy(&hop_m);
}

