/* -*- c++ -*- */
/*
 * Copyright 2006,2012 Free Software Foundation, Inc.
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
 */

#include "agc.h"

void agc(axis_float in[SIZE], axis_float out[SIZE])
{
#pragma HLS INTERFACE depth=1 axis port=in
#pragma HLS INTERFACE depth=1 axis port=out
#pragma HLS INTERFACE ap_ctrl_none port=return

	static agc_ff agc;
	float agc_in[SIZE];

	for(int i = 0; i < SIZE; i++) {
		agc_in[i] = in[i].data;
	}

	for(int i = 0; i < SIZE; i++) {
		out[i].data = agc.scale(agc_in[i]);
		out[i].last = in[i].last;
	}
}
