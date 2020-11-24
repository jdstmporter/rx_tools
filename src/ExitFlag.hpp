/*
 * ExitFlag.hpp
 *
 *  Created on: 23 Nov 2020
 *      Author: julianporter
 */

#ifndef SRC_EXITFLAG_HPP_
#define SRC_EXITFLAG_HPP_



#include <atomic>

class ExitFlag {
private:
	std::atomic_bool flag;

public:
	ExitFlag();
	virtual ~ExitFlag() = default;

	void set();
	void clear();

	operator bool();
};

ExitFlag doExit;

#endif /* SRC_EXITFLAG_HPP_ */
