/*
 * Demod.hpp
 *
 *  Created on: 22 Nov 2020
 *      Author: julianporter
 */

#ifndef DEMOD_HPP_
#define DEMOD_HPP_

#include "base.hpp"

struct output_state;


struct demod_state
{
	int	  exit_flag;
	pthread_t thread;
	int16_t  lowpassed[MAXIMUM_BUF_LENGTH];
	int	  lp_len;
	int16_t  lp_i_hist[10][6];
	int16_t  lp_q_hist[10][6];
	int16_t  result[MAXIMUM_BUF_LENGTH];
	int16_t  droop_i_hist[9];
	int16_t  droop_q_hist[9];
	int	  result_len;
	int	  rate_in;
	int	  rate_out;
	int	  rate_out2;
	int	  now_r, now_j;
	int	  pre_r, pre_j;
	int	  prev_index;
	int	  downsample;	/* min 1, max 256 */
	int	  post_downsample;
	int	  output_scale;
	int	  squelch_level, conseq_squelch, squelch_hits, terminate_on_squelch, squelch_zero;
	int	  downsample_passes;
	int	  comp_fir_size;
	int	  custom_atan;
	int	  deemph, deemph_a;
	int	  now_lpr;
	int	  prev_lpr_index;
	int	  dc_block_audio, dc_avg, adc_block_const;
	int	  dc_block_raw, dc_avgI, dc_avgQ, rdc_block_const;
	void	 (*mode_demod)(struct demod_state*);
	pthread_rwlock_t rw;
	pthread_cond_t ready;
	pthread_mutex_t ready_m;
	struct output_state *output_target;
};


struct Demodulator;
struct Output;
struct Controller;

struct Demod {
	int	  exit_flag = 0;
	pthread_t thread = 0;
	int16_t  lowpassed[MAXIMUM_BUF_LENGTH];
	int	  lp_len = 0;
	int16_t  lp_i_hist[10][6];
	int16_t  lp_q_hist[10][6];
	int16_t  result[MAXIMUM_BUF_LENGTH];
	int16_t  droop_i_hist[9];
	int16_t  droop_q_hist[9];
	int	  result_len = 0;
	int	  rate_in = DEFAULT_SAMPLE_RATE;
	int	  rate_out = DEFAULT_SAMPLE_RATE;
	int	  rate_out2 = -1;
	int	  now_r = 0;
	int   now_j = 0;
	int	  pre_r = 0;
	int   pre_j = 0;
	int	  prev_index = 0;
	int	  downsample = 1;	/* min 1, max 256 */
	int	  post_downsample = 1;
	int	  output_scale = 0;
	int	  squelch_level = 0;
	int   conseq_squelch = 10;
	int   squelch_hits=11;
	int   terminate_on_squelch = 0;
	bool   squelch_zero = false;
	int	  downsample_passes = 0;
	int	  comp_fir_size = 0;
	int	  custom_atan = 0;
	bool	  deemph = false;
	int   deemph_a = 0;
	int	  now_lpr = 0;
	int	  prev_lpr_index = 0;
	bool	  dc_block_audio = false;
	int   dc_avg = 0;
	int   adc_block_const = 0;
	int	  dc_block_raw = 0;
	int   dc_avgI = 0;
	int   dc_avgQ = 0;
	int   rdc_block_const = 0;
	std::shared_ptr<Demodulator> mode_demod;
	pthread_rwlock_t rw;
	pthread_cond_t ready;
	pthread_mutex_t ready_m;
	Output *output_target;

	Demod(output_state *output = nullptr);
	virtual ~Demod();

	void low_pass();
	void deemph_filter();
	void dc_block_audio_filter();
	void low_pass_real();
	void dc_block_raw_filter(int16_t *buf,int len);
	void full_demod();
	void threadFunction(Controller *);
};

#endif /* DEMOD_HPP_ */
