/*
 * Demod.cpp
 *
 *  Created on: 22 Nov 2020
 *      Author: julianporter
 */

#include "Demod.hpp"
#include "filters.hpp"
#include "demodulators.hpp"
#include "SDR.hpp"
#include "Output.hpp"
#include "Controller.hpp"


Demod::Demod() {
	mode_demod=std::make_shared<FMDemodulator>();
	pthread_rwlock_init(&rw, NULL);
	pthread_cond_init(&ready, NULL);
	pthread_mutex_init(&ready_m, NULL);
}


Demod::~Demod()
{
	pthread_rwlock_destroy(&rw);
	pthread_cond_destroy(&ready);
	pthread_mutex_destroy(&ready_m);
}

void Demod::dc_block_raw_filter(int16_t *buf,int len) {
	/* derived from dc_block_audio_filter,
		running over the raw I/Q components
	 */
	int i, avgI, avgQ;
	int64_t sumI = 0;
	int64_t sumQ = 0;
	for (i = 0; i < len; i += 2) {
		sumI += buf[i];
		sumQ += buf[i+1];
	}
	avgI = sumI / ( len / 2 );
	avgQ = sumQ / ( len / 2 );
	avgI = (avgI + dc_avgI * rdc_block_const) / ( rdc_block_const + 1 );
	avgQ = (avgQ + dc_avgQ * rdc_block_const) / ( rdc_block_const + 1 );
	for (i = 0; i < len; i += 2) {
		buf[i] -= avgI;
		buf[i+1] -= avgQ;
	}
	dc_avgI = avgI;
	dc_avgQ = avgQ;
}


void Demod::low_pass()
/* simple square window FIR */
{
	int i=0, i2=0;
	while (i < lp_len) {
		now_r += lowpassed[i];
		now_j += lowpassed[i+1];
		i += 2;
		prev_index++;
		if (prev_index < downsample) {
			continue;
		}
		lowpassed[i2]   = now_r; // * output_scale;
		lowpassed[i2+1] = now_j; // * output_scale;
		prev_index = 0;
		now_r = 0;
		now_j = 0;
		i2 += 2;
	}
	lp_len = i2;
}

void Demod::deemph_filter()
{
	static int avg;  // cheating...
	int i, d;
	// de-emph IIR
	// avg = avg * (1 - alpha) + sample * alpha;
	for (i = 0; i < result_len; i++) {
		d = result[i] - avg;
		if (d > 0) {
			avg += (d + deemph_a/2) / deemph_a;
		} else {
			avg += (d - deemph_a/2) / deemph_a;
		}
		result[i] = (int16_t)avg;
	}
}

void Demod::dc_block_audio_filter()
{
	int i, avg;
	int64_t sum = 0;
	for (i=0; i < result_len; i++) {
		sum += result[i];
	}
	avg = sum / result_len;
	avg = (avg + dc_avg * adc_block_const) / ( adc_block_const + 1 );
	for (i=0; i < result_len; i++) {
		result[i] -= avg;
	}
	dc_avg = avg;
}

void Demod::low_pass_real()
/* simple square window FIR */
// add support for upsampling?
{
	int i=0, i2=0;
	int fast = (int)rate_out;
	int slow = rate_out2;
	while (i < result_len) {
		now_lpr += result[i];
		i++;
		prev_lpr_index += slow;
		if (prev_lpr_index < fast) {
			continue;
		}
		result[i2] = (int16_t)(now_lpr / (fast/slow));
		prev_lpr_index -= fast;
		now_lpr = 0;
		i2 += 1;
	}
	result_len = i2;
}

void Demod::full_demod() {
		int i;
		int sr = 0;
		if (downsample_passes>0) {
			for (i=0; i < downsample_passes; i++) {
				fifth_order(lowpassed,   (lp_len >> i), lp_i_hist[i]);
				fifth_order(lowpassed+1, (lp_len >> i) - 1, lp_q_hist[i]);
			}
			lp_len = lp_len >> downsample_passes;
			/* droop compensation */
			if (comp_fir_size == 9 && downsample_passes <= CIC_TABLE_MAX) {
				generic_fir(lowpassed, lp_len,
					cic_9_tables[downsample_passes], droop_i_hist);
				generic_fir(lowpassed+1, lp_len-1,
					cic_9_tables[downsample_passes], droop_q_hist);
			}
		} else {
			low_pass();
		}
		/* power squelch */
		if (squelch_level>0) {
			sr = rms(lowpassed, lp_len, 1);
			if (sr < squelch_level) {
				squelch_hits++;
				for (i=0; i<lp_len; i++) {
					lowpassed[i] = 0;
				}
			} else {
				squelch_hits = 0;}
		}

		/*
		if (printLevels) {
			if (!sr)
				sr = rms(lowpassed, lp_len, 1);
			--printLevelNo;
			if (printLevels) {
				levelSum += sr;
				if (levelMax < sr)		levelMax = sr;
				if (levelMaxMax < sr)	levelMaxMax = sr;
				if  (!printLevelNo) {
					printLevelNo = printLevels;
					fprintf(stderr, "%f, %d, %d, %d\n", (levelSum / printLevels), levelMax, levelMaxMax, d->squelch_level );
					levelMax = 0;
					levelSum = 0;
				}
			}
		}
		*/
		mode_demod->demodulate(this);  /* lowpassed -> result */
		if (mode_demod->kind == Demodulator::Kind::RAW) {
			return;
		}
		/* todo, fm noise squelch */
		// use nicer filter here too?
		if (post_downsample > 1) {
			result_len = low_pass_simple(result, result_len, post_downsample);}
		if (deemph) deemph_filter();
		if (dc_block_audio) dc_block_audio_filter();
		if (rate_out2 > 0) {
			low_pass_real();
			//arbitrary_resample(d->result, d->result, d->result_len, d->result_len * d->rate_out2 / d->rate_out);
		}


}

void Demod::threadFunction() {
		auto controller = SDR::shared()->controller;
		auto output_target = SDR::shared()->output;
		while (!doExit) {
			safe_cond_wait(&ready, &ready_m);
			pthread_rwlock_wrlock(&rw);
			full_demod();
			pthread_rwlock_unlock(&rw);
			if (exit_flag) {
				doExit.set();
			}
			bool squelch_active = (squelch_level > 0 && squelch_hits > conseq_squelch);
			if (squelch_active && !squelch_zero) {
				squelch_hits = conseq_squelch + 1;  /* hair trigger */
				safe_cond_signal(&controller->hop, &controller->hop_m);
				continue;
			}
			pthread_rwlock_wrlock(&output_target->rw);
			if (squelch_active && squelch_zero) {
				memset(output_target->result, 0, 2*result_len);
			} else {
				memcpy(output_target->result, result, 2*result_len);
			}
			output_target->result_len = result_len;
			pthread_rwlock_unlock(&output_target->rw);
			safe_cond_signal(&output_target->ready, &output_target->ready_m);
		}
}
