/*
 * filters.hpp
 *
 *  Created on: 23 Nov 2020
 *      Author: julianporter
 */

#ifndef SRC_FILTERS_HPP_
#define SRC_FILTERS_HPP_

#include "base.hpp"
#define CIC_TABLE_MAX 10
int cic_9_tables[][CIC_TABLE_MAX];

void fifth_order(int16_t *data, int length, int16_t *hist);
void generic_fir(int16_t *data, int length, int *fir, int16_t *hist);
int low_pass_simple(int16_t *signal2, int len, int step);

int mad(int16_t *samples, int len, int step);
int rms(int16_t *samples, int len, int step);


#endif /* SRC_FILTERS_HPP_ */
