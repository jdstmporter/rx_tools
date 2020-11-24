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
		AM,
		LSB,
		USB,
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

struct AMDemodulator : public Demodulator {
	AMDemodulator() : Demodulator(Kind::AM) {};
	virtual void demodulate(Demod *) {}
};

struct LSBDemodulator : public Demodulator {
	LSBDemodulator() : Demodulator(Kind::LSB) {};
	virtual void demodulate(Demod *) {}
};

struct USBDemodulator : public Demodulator {
	USBDemodulator() : Demodulator(Kind::USB) {};
	virtual void demodulate(Demod *) {}
};

struct RawDemodulator : public Demodulator {
	RawDemodulator() : Demodulator(Kind::RAW) {};
	virtual void demodulate(Demod *) {}
};

#endif /* SRC_DEMODULATORS_HPP_ */
