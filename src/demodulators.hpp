/*
 * demodulators.hpp
 *
 *  Created on: 23 Nov 2020
 *      Author: julianporter
 */

#ifndef SRC_DEMODULATORS_HPP_
#define SRC_DEMODULATORS_HPP_

#include "base.hpp"

struct Demod;


struct Demodulator {

	enum class Kind {
		FM,
		RAW,
		NUL
	};

	Kind kind;
	Demodulator(Kind kind_ = Kind::NUL) : kind(kind_) {};
	virtual ~Demodulator() = default;

	virtual void demodulate(Demod *) = 0;
};
struct FMDemodulator : public Demodulator {

	FMDemodulator() : Demodulator(Kind::FM) {};
	virtual void demodulate(Demod *) {}
};

#endif /* SRC_DEMODULATORS_HPP_ */
