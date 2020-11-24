/*
 * SDR.cpp
 *
 *  Created on: 23 Nov 2020
 *      Author: julianporter
 */

#include "SDR.hpp"

;

SDR::SDR() : output(), demod(), dongle(), controller() {};

void SDR::activate() {
	output=std::make_shared<Output>();
	demod=std::make_shared<Demod>();
	dongle=std::make_shared<Dongle>();
	controller=std::make_shared<Controller>();
}

std::shared_ptr<SDR> SDR::shared() {
	if(!_shared) {
		_shared=std::make_shared<SDR>();
		_shared->activate();
	}
	return _shared;
}
