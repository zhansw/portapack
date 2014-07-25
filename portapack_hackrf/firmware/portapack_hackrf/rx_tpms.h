/*
 * Copyright (C) 2014 Jared Boone, ShareBrained Technology, Inc.
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

#ifndef __RX_TPMS_H__
#define __RX_TPMS_H__

#include <stddef.h>

#include "portapack.h"

#include "complex.h"

#include "packet_builder.h"

void rx_tpms_init(void* const _state, packet_builder_payload_handler_t payload_handler);
void rx_tpms_baseband_handler(void* const _state, complex_s8_t* const in, const size_t sample_count_in, void* const out, baseband_timestamps_t* const timestamps);

#endif/*__RX_TPMS_H__*/
