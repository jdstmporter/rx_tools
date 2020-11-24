/*
 * Controller.cpp
 *
 *  Created on: 23 Nov 2020
 *      Author: julianporter
 */

#include "Controller.hpp"
#include "Demod.hpp"
#include "convenience.h"
#include "SDR.hpp"

void usage(void)
{
	fprintf(stderr,
		"rx_fm (based on rtl_fm), a simple narrow band FM demodulator for RTL2832 based DVB-T receivers\n\n"
		"Use:\trx_fm -f freq [-options] [filename]\n"
		"\t-f frequency_to_tune_to [Hz]\n"
		"\t	use multiple -f for scanning (requires squelch)\n"
		"\t	ranges supported, -f 118M:137M:25k\n"
		"\t[-v increase verbosity (default: 0)]\n"
		"\t[-M modulation (default: fm)]\n"
		"\t	fm or nbfm or nfm, wbfm or wfm, raw or iq, am, usb, lsb\n"
		"\t	wbfm == -M fm -s 170k -o 4 -A fast -r 32k -l 0 -E deemp\n"
		"\t	raw mode outputs 2x16 bit IQ pairs\n"
		"\t[-s sample_rate (default: 24k)]\n"
		"\t[-d device key/value query (ex: 0, 1, driver=rtlsdr, driver=hackrf)]\n"
		"\t[-g tuner gain(s) (ex: 20, 40, LNA=40,VGA=20,AMP=0)]\n"
		"\t[-w tuner_bandwidth (default: automatic. enables offset tuning)]\n"
		"\t[-C channel number (ex: 0)]\n"
		"\t[-a antenna (ex: 'Tuner 1 50 ohm')]\n"
		"\t[-l squelch_level (default: 0/off)]\n"
		"\t[-L N  prints levels every N calculations]\n"
		"\t	output are comma separated values (csv):\n"
		"\t	mean since last output, max since last output, overall max, squelch\n"
		"\t[-c de-emphasis_time_constant in us for wbfm. 'us' or 'eu' for 75/50 us (default: us)]\n"
		//"\t	for fm squelch is inverted\n"
		"\t[-o oversampling (default: 1, 4 recommended)]\n"
		"\t[-p ppm_error (default: 0)]\n"
		"\t[-E enable_option (default: none)]\n"
		"\t	use multiple -E to enable multiple options\n"
		"\t	edge:   enable lower edge tuning\n"
		"\t	rdc:    enable dc blocking filter on raw I/Q data at capture rate\n"
		"\t	adc:    enable dc blocking filter on demodulated audio\n"
		"\t	dc:     same as adc\n"
		"\t	rtlagc: enable rtl2832's digital agc (default: off)\n"
		"\t	agc:    same as rtlagc\n"
		"\t	deemp:  enable de-emphasis filter\n"
		"\t	direct: enable direct sampling (bypasses tuner, uses rtl2832 xtal)\n"
		"\t	no-mod: enable no-mod direct sampling\n"
		"\t	offset: enable offset tuning (only e4000 tuner)\n"
		"\t	zero:   emit zeros when squelch active\n"
		"\t	wav:    generate WAV header\n"
		"\t[-q dc_avg_factor for option rdc (default: 9)]\n"
		"\tfilename ('-' means stdout)\n"
		"\t	omitting the filename also uses stdout\n\n"
		"Experimental options:\n"
		"\t[-r resample_rate (default: none / same as -s)]\n"
		"\t[-t squelch_delay (default: 10)]\n"
		"\t	+values will mute/scan, -values will exit\n"
		"\t[-F fir_size (default: off)]\n"
		"\t	enables low-leakage downsample filter\n"
		"\t	size can be 0 or 9.  0 has bad roll off\n"
		"\t[-A std/fast/lut/ale choose atan math (default: std)]\n"
		//"\t[-C clip_path (default: off)\n"
		//"\t (create time stamped raw clips, requires squelch)\n"
		//"\t (path must have '\%s' and will expand to date_time_freq)\n"
		//"\t[-H hop_fifo (default: off)\n"
		//"\t (fifo will contain the active frequency)\n"
		"\n"
		"Produces signed 16 bit ints, use Sox or aplay to hear them.\n"
		"\trx_fm ... | play -t raw -r 24k -es -b 16 -c 1 -V1 -\n"
		"\t		   | aplay -r 24k -f S16_LE -t raw -c 1\n"
		"\t  -M wbfm  | play -r 32k ... \n"
		"\t  -E wav   | play -t wav - \n"
		"\t  -s 22050 | multimon -t raw /dev/stdin\n\n");
	exit(1);
}

static int lcm_post[17] = {1,1,1,3,1,5,3,7,1,9,5,11,3,13,7,15,1};


Controller::Controller() {
	freqs[0] = 100000000;
	pthread_cond_init(&hop, NULL);
	pthread_mutex_init(&hop_m, NULL);
}

Controller::~Controller() {
	pthread_cond_destroy(&hop);
	pthread_mutex_destroy(&hop_m);
}

int Controller::ACTUAL_BUF_LENGTH() {
	return lcm_post[SDR::shared()->demod->post_downsample]*DEFAULT_BUF_LENGTH;
}

void Controller::frequency_range( char *arg)
{
	char *start, *stop, *step;
	int i;
	start = arg;
	stop = strchr(start, ':') + 1;
	stop[-1] = '\0';
	step = strchr(stop, ':') + 1;
	step[-1] = '\0';
	for(i=(int)atofs(start); i<=(int)atofs(stop); i+=(int)atofs(step))
	{
		freqs[freq_len] = (uint32_t)i;
		freq_len++;
		if (freq_len >= FREQUENCIES_LIMIT) {
			break;}
	}
	stop[-1] = ':';
	step[-1] = ':';
}

void Controller::sanity_checks(void)
{
	if (freq_len == 0) {
		fprintf(stderr, "Please specify a frequency.\n");
		usage();
	}

	if (freq_len >= FREQUENCIES_LIMIT) {
		fprintf(stderr, "Too many channels, maximum %i.\n", FREQUENCIES_LIMIT);
		exit(1);
	}

	if (freq_len > 1 && SDR::shared()->demod->squelch_level == 0) {
		fprintf(stderr, "Please specify a squelch level.  Required for scanning multiple frequencies.\n");
		exit(1);
	}

}

void Controller::optimal_settings(int freq, int rate)
{
	// giant ball of hacks
	// seems unable to do a single pass, 2:1
	int capture_freq, capture_rate;
	auto dm = SDR::shared()->demod;
	auto dongle = SDR::shared()->dongle;
	dm->downsample = (1000000 / dm->rate_in) + 1;
	if (dm->downsample_passes) {
		dm->downsample_passes = (int)log2(dm->downsample) + 1;
		dm->downsample = 1 << dm->downsample_passes;
	}
	if (verbosity) {
		fprintf(stderr, "downsample_passes = %d (= # of fifth_order() iterations), downsample = %d\n", dm->downsample_passes, dm->downsample );
	}
	capture_freq = freq;
	capture_rate = dm->downsample * dm->rate_in;
	if (verbosity)
		fprintf(stderr, "capture_rate = dm->downsample * dm->rate_in = %d * %d = %d\n", dm->downsample, dm->rate_in, capture_rate );
	if (!dongle->offset_tuning) {
		capture_freq = freq + capture_rate/4;
		if (verbosity)
			fprintf(stderr, "optimal_settings(freq = %d): capture_freq = freq + capture_rate/4 = %d\n", freq, capture_freq );
	}
	capture_freq += edge * dm->rate_in / 2;
	if (verbosity)
		fprintf(stderr, "optimal_settings(freq = %d): capture_freq +=  cs->edge * dm->rate_in / 2 = %d * %d / 2 = %d\n", freq, edge, dm->rate_in, capture_freq );
	dm->output_scale = (1<<15) / (128 * dm->downsample);
	if (dm->output_scale < 1) {
		dm->output_scale = 1;}
	if (dm->demodKind() == Demodulator::Kind::FM) {
		dm->output_scale = 1;}
	dongle->freq = (uint32_t)capture_freq;
	dongle->rate = (uint32_t)capture_rate;
	if (verbosity)
		fprintf(stderr, "optimal_settings(freq = %d) delivers freq %.0f, rate %.0f\n", freq, (double)dongle->freq, (double)dongle->rate );
}

void Controller::threadFunction()
{
	auto demod=SDR::shared()->demod;
	auto dongle = SDR::shared()->dongle;
	// thoughts for multiple dongles
	// might be no good using a controller thread if retune/rate blocks
	int i;

	if (wb_mode) {
		if (verbosity)
			fprintf(stderr, "wbfm: adding 16000 Hz to every input frequency\n");
		for (i=0; i < freq_len; i++) {
			freqs[i] += 16000;}
	}

	/* set up primary channel */
	optimal_settings(freqs[0], demod->rate_in);
	if (dongle->direct_sampling) {
		verbose_direct_sampling(dongle->dev, dongle->direct_sampling);}
	if (dongle->offset_tuning) {
		verbose_offset_tuning(dongle->dev);}

	/* Set the frequency */
	if (verbosity) {
		fprintf(stderr, "verbose_set_frequency(%.0f Hz)\n", (double)dongle->freq);
		if (!dongle->offset_tuning)
			fprintf(stderr, "  frequency is away from parametrized one, to avoid negative impact from dc\n");
	}
	verbose_set_frequency(dongle->dev, dongle->freq, dongle->channel);
	fprintf(stderr, "Oversampling input by: %ix.\n", demod->downsample);
	fprintf(stderr, "Oversampling output by: %ix.\n", demod->post_downsample);
	fprintf(stderr, "Buffer size: %0.2fms\n",
		1000 * 0.5 * (float)ACTUAL_BUF_LENGTH() / (float)dongle->rate);

	/* Set the sample rate */
	if (verbosity)
		fprintf(stderr, "verbose_set_sample_rate(%.0f Hz)\n", (double)dongle->rate);
	verbose_set_sample_rate(dongle->dev, dongle->rate, dongle->channel);
	fprintf(stderr, "Output at %u Hz.\n", demod->rate_in/demod->post_downsample);

	SoapySDRKwargs args = {0};
	while (!doExit) {
		safe_cond_wait(&hop, &hop_m);
		if (freq_len <= 1) {
			continue;}
		/* hacky hopping */
		freq_now = (freq_now + 1) % freq_len;
		optimal_settings(freqs[freq_now], demod->rate_in);
		SoapySDRDevice_setFrequency(dongle->dev, SOAPY_SDR_RX, 0, (double)dongle->freq, &args);
		dongle->mute = BUFFER_DUMP;
	}
}

