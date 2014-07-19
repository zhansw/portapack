/*
 * Copyright (C) 2013 Jared Boone, ShareBrained Technology, Inc.
 *
 * This file is part of PortaPack.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifndef __PORTAPACK_H__
#define __PORTAPACK_H__

#include <stdint.h>

#include "ipc.h"

typedef struct dsp_metrics_t {
	uint32_t duration_decimate;
	uint32_t duration_channel_filter;
	uint32_t duration_demodulate;
	uint32_t duration_audio;
	uint32_t duration_all;
	uint32_t duration_all_millipercent;
} dsp_metrics_t;

typedef struct device_state_t {
	int64_t tuned_hz;
	int32_t lna_gain_db;
	int32_t if_gain_db;
	int32_t bb_gain_db;
	int32_t audio_out_gain_db;
	int32_t receiver_configuration_index;
	
	int32_t encoder_position;

	ipc_channel_t ipc_m4;
	ipc_channel_t ipc_m0;

	dsp_metrics_t dsp_metrics;
} device_state_t;

void portapack_init();
void portapack_run();

#endif/*__PORTAPACK_H__*/
