/* -*- c++ -*- */
/*
 * Copyright 2013 Free Software Foundation, Inc.
 *
 * This file is part of GNU Radio
 *
 * GNU Radio is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * GNU Radio is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 *
 * Modified by Andrew Lanez and Sachin Bharadwaj Sundramurthy (Team Rabbit Ears)
 * for RFNoC and Vivado HLS Challenge
 */

#include <complex>
#include <ap_int.h>

#define IN_SIZE 	64
#define OUT_SIZE 	122	//2*ceil(1.8941538461538463*IN_SIZE/2)
#define TRANSIENT	18 	//Overlap/discard length of output filter arm's transient

#define RRC_NTAPS 	299
#define FILTER_SIZE	16
#define INTERP		1.894153846153846298960843341774307191371917724609375
#define FLT_RATE	FILTER_SIZE/INTERP // residual rate for the linear interpolation
#define ARMS		32
#define TAPS		19
#define M_2PI		2*M_PI

struct axis_float {
	float data;
    ap_uint<1> last;
};

void atsc_rx_filter(axis_float in[IN_SIZE], axis_float out[OUT_SIZE]);

/*!
* \brief Polyphase filterbank arbitrary resampler with
*        complex input, complex output and float taps
* \ingroup resamplers_blk
*
* \details
* This  takes in a signal stream and performs arbitrary
* resampling. The resampling rate can be any real number
* <EM>r</EM>. The resampling is done by constructing <EM>N</EM>
* filters where <EM>N</EM> is the interpolation rate.  We then
* calculate <EM>D</EM> where <EM>D = floor(N/r)</EM>.
*
* Using <EM>N</EM> and <EM>D</EM>, we can perform rational
* resampling where <EM>N/D</EM> is a rational number close to
* the input rate <EM>r</EM> where we have <EM>N</EM> filters
* and we cycle through them as a polyphase filterbank with a
* stride of <EM>D</EM> so that <EM>i+1 = (i + D) % N</EM>.
*
* To get the arbitrary rate, we want to interpolate between two
* points. For each value out, we take an output from the
* current filter, <EM>i</EM>, and the next filter <EM>i+1</EM>
* and then linearly interpolate between the two based on the
* real resampling rate we want.
*
* The linear interpolation only provides us with an
* approximation to the real sampling rate specified. The error
* is a quantization error between the two filters we used as
* our interpolation points.  To this end, the number of
* filters, <EM>N</EM>, used determines the quantization error;
* the larger <EM>N</EM>, the smaller the noise. You can design
* for a specified noise floor by setting the filter size
* (parameters <EM>filter_size</EM>). The size defaults to 32
* filters, which is about as good as most implementations need.
*
* The trick with designing this filter is in how to specify the
* taps of the prototype filter. Like the PFB interpolator, the
* taps are specified using the interpolated filter rate. In
* this case, that rate is the input sample rate multiplied by
* the number of filters in the filterbank, which is also the
* interpolation rate. All other values should be relative to
* this rate.
*
* For example, for a 32-filter arbitrary resampler and using
* the GNU Radio's firdes utility to build the filter, we build
* a low-pass filter with a sampling rate of <EM>fs</EM>, a 3-dB
* bandwidth of <EM>BW</EM> and a transition bandwidth of
* <EM>TB</EM>. We can also specify the out-of-band attenuation
* to use, <EM>ATT</EM>, and the filter window function (a
* Blackman-harris window in this case). The first input is the
* gain of the filter, which we specify here as the
* interpolation rate (<EM>32</EM>).
*
*   <B><EM>self._taps = filter.firdes.low_pass_2(32, 32*fs, BW, TB,
*      attenuation_dB=ATT, window=filter.firdes.WIN_BLACKMAN_hARRIS)</EM></B>
*
* The theory behind this block can be found in Chapter 7.5 of
* the following book.
*
*   <B><EM>f. harris, "Multirate Signal Processing for Communication
*      Systems", Upper Saddle River, NJ: Prentice Hall, Inc. 2004.</EM></B>
*/

/*!
 * Creates a kernel to perform arbitrary resampling on a set of samples.
 *  The number of filters in the filter bank is directly
 *                    related to quantization noise introduced during the resampling.
 *                    Defaults to 32 filters.
 */

/*!
 * Sets the current phase offset in filterbank.
 */
void set_phase(float ph);

/*!
 * Gets the current phase of the resampler filterbank.
 */
float phase();

/*!
 * Performs the filter operation that resamples the signal.
 *
 * This block takes in a stream of samples and outputs a
 * resampled and filtered stream. This block should be called
 * such that the output has \p rate * \p n_to_read amount of
 * space available in the \p output buffer.
 */
int filter(std::complex<float> *out, std::complex<float> *in,
		   int n_to_read);

void set_acc(float acc);

float acc();

float mod(float a, float b);

std::complex<float> filters(std::complex<float> input[], int j);
std::complex<float> diff_filters(std::complex<float> input[], int j);
