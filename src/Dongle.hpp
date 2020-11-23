/*
 * Dongle.hpp
 *
 *  Created on: 22 Nov 2020
 *      Author: julianporter
 */

#ifndef SRC_DONGLE_HPP_
#define SRC_DONGLE_HPP_

#include "base.hpp"

struct demod_state;
struct Demod;

struct dongle_state
{
	int	  exit_flag;
	pthread_t thread;
	SoapySDRDevice *dev;
	SoapySDRStream *stream;
	size_t channel;
	char	*dev_query;
	uint32_t freq;
	uint32_t rate;
	uint32_t bandwidth;
	char *gain_str;
	int16_t  buf16[MAXIMUM_BUF_LENGTH];
	int	  ppm_error;
	int	  offset_tuning;
	int	  direct_sampling;
	int	  mute;
	demod_state *demod_target;



};

struct Dongle {
	int	  exit_flag = 0;
	pthread_t thread = 0;
	SoapySDRDevice *dev = nullptr;
	SoapySDRStream *stream = nullptr;
	size_t channel = 0;
	char	*dev_query = nullptr;
	uint32_t freq = 0;
	uint32_t rate = DEFAULT_SAMPLE_RATE;
	uint32_t bandwidth = 0;
	char *gain_str = nullptr;
	int16_t  buf16[MAXIMUM_BUF_LENGTH];
	int	  ppm_error = 0;
	bool	  offset_tuning = false;
	bool	  direct_sampling = false;
	int	  mute = 0;
	Demod *demod_target;

	void rotate16_90(uint32_t len);

	Dongle(demod_state *demod = nullptr) : demod_target(demod) {};
	virtual ~Dongle() = default;

	size_t samplesPerBuffer() const { return MAXIMUM_BUF_LENGTH/2; }

	void activateStream() {
		SoapySDRDevice_activateStream(dev,stream,0,0,0);
	}
	int read(void *buffs[],int *flags,long long *time,long timeout=1000000) {
		 return SoapySDRDevice_readStream(
				 dev,stream, buffs,  samplesPerBuffer(), flags, time, timeout);
	}
	void disactivateStream() {
		SoapySDRDevice_deactivateStream(dev,stream,0,0);
	}


	void unloadBuffer(int16_t *target,uint32_t len) {
		memcpy(target, buf16, 2*len);
	}

	void rotate16_90(uint32_t len) {

	}

	void process(int16_t *buf,uint32_t len);
};



#endif /* SRC_DONGLE_HPP_ */
