/*
 * ExitFlag.cpp
 *
 *  Created on: 23 Nov 2020
 *      Author: julianporter
 */

#include "ExitFlag.hpp"


	ExitFlag::ExitFlag() : flag(false) {};

	void ExitFlag::set() { flag.store(true); }
	void ExitFlag::clear() { flag.store(false); }

	ExitFlag::operator bool() { return flag.load(); }


