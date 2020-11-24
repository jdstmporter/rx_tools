/*
 * SDR.hpp
 *
 *  Created on: 23 Nov 2020
 *      Author: julianporter
 */

#ifndef SRC_SDR_HPP_
#define SRC_SDR_HPP_

#include "base.hpp"
#include "Output.hpp"
#include "Demod.hpp"
#include "Dongle.hpp"
#include "Controller.hpp"

class SDR {
private:
	void activate();
	static std::shared_ptr<SDR> _shared;

public:

	std::shared_ptr<Output> output;
	std::shared_ptr<Demod> demod;
	std::shared_ptr<Dongle> dongle;
	std::shared_ptr<Controller> controller;



	SDR();
	SDR(const SDR&) = default;
	SDR & operator=(const SDR&) = default;
	virtual ~SDR();



	static std::shared_ptr<SDR> shared();

};



#endif /* SRC_SDR_HPP_ */
