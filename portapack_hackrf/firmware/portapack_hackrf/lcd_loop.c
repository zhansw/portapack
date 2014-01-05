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

#include <stdint.h>

#include "lcd.h"
#include "font_fixed_8x16.h"

#include "lcd_loop.h"

#include <stdio.h>

#include "ipc.h"
#include "linux_stuff.h"

void delay(uint32_t duration)
{
	uint32_t i;
	for (i = 0; i < duration; i++)
		__asm__("nop");
}

static void draw_int(int32_t value, const char* const format, uint_fast16_t x, uint_fast16_t y) {
	char temp[80];
	const size_t temp_len = 79;
	const size_t text_len = snprintf(temp, temp_len, format, value);
	lcd_draw_string(x, y, temp, min(text_len, temp_len));
}

static void draw_mhz(int64_t value, const char* const format, uint_fast16_t x, uint_fast16_t y) {
	char temp[80];
	const size_t temp_len = 79;
	const int32_t value_mhz = value / 1000000;
	const int32_t value_hz = (value - (value_mhz * 1000000)) / 1000;
	const size_t text_len = snprintf(temp, temp_len, format, value_mhz, value_hz);
	lcd_draw_string(x, y, temp, min(text_len, temp_len));
}

static void draw_percent(int32_t value_millipercent, const char* const format, uint_fast16_t x, uint_fast16_t y) {
	char temp[80];
	const size_t temp_len = 79;
	const int32_t value_units = value_millipercent / 1000;
	const int32_t value_millis = (value_millipercent - (value_units * 1000)) / 100;
	const size_t text_len = snprintf(temp, temp_len, format, value_units, value_millis);
	lcd_draw_string(x, y, temp, min(text_len, temp_len));
}

static void draw_cycles(const uint_fast16_t x, const uint_fast16_t y) {
	lcd_colors_invert();
	lcd_draw_string(x, y, "Cycle Count ", 12);
	lcd_colors_invert();

	draw_int(device_state->duration_decimate,       "Decim %6d", x, y + 16);
	draw_int(device_state->duration_channel_filter, "Chan  %6d", x, y + 32);
	draw_int(device_state->duration_demodulate,     "Demod %6d", x, y + 48);
	draw_int(device_state->duration_audio,          "Audio %6d", x, y + 64);
	draw_int(device_state->duration_all,            "Total %6d", x, y + 80);
	draw_percent(device_state->duration_all_millipercent, "CPU   %3d.%01d%%", x, y + 96);
}

struct ui_field_text_t;
typedef struct ui_field_text_t ui_field_text_t;

struct ui_field_text_t {
	uint_fast16_t x;
	uint_fast16_t y;
	const char* const format;
	const void* (*getter)();
	void (*render)(const ui_field_text_t* const);
};

static void render_field_mhz(const ui_field_text_t* const field) {
	const int64_t value = *((int64_t*)field->getter());
	draw_mhz(value, field->format, field->x, field->y);
}

static void render_field_int(const ui_field_text_t* const field) {
	const int32_t value = *((int32_t*)field->getter());
	draw_int(value, field->format, field->x, field->y);
}

static const void* get_tuned_hz() {
	return &device_state->tuned_hz;
}

static const void* get_lna_gain() {
	return &device_state->lna_gain_db;
}

static const void* get_if_gain() {
	return &device_state->if_gain_db;
}

static const void* get_bb_gain() {
	return &device_state->bb_gain_db;
}

static ui_field_text_t fields[] = {
	{ .x = 0, .y = 32, .format = "%4d.%03d MHz", .getter = get_tuned_hz, .render = render_field_mhz },
	{ .x = 0, .y = 64, .format = "LNA %2d dB",   .getter = get_lna_gain, .render = render_field_int },
	{ .x = 0, .y = 80, .format = "IF  %2d dB",   .getter = get_if_gain,  .render = render_field_int },
	{ .x = 0, .y = 96, .format = "BB  %2d dB",   .getter = get_bb_gain,  .render = render_field_int }
};

static void render_fields(const ui_field_text_t* const fields, const size_t count) {
	for(size_t i=0; i<count; i++) {
		fields[i].render(&fields[i]);
	}
}

static void handle_joysticks() {
	static uint32_t switches_history[3] = { 0, 0, 0 };
	static uint32_t switches_last = 0;

	const uint32_t switches_raw = lcd_data_read_switches();
	uint32_t switches_now = switches_raw & switches_history[0] & switches_history[1] & switches_history[2];
	switches_history[0] = switches_history[1];
	switches_history[1] = switches_history[2];
	switches_history[2] = switches_raw;

	const uint32_t switches_event = switches_now ^ switches_last;
	const uint32_t switches = switches_event & switches_now;
	switches_last = switches_now;

	const uint_fast8_t switches_incr
		= ((switches & SWITCH_S1_LEFT) ? 8 : 0)
		| ((switches & SWITCH_S2_LEFT) ? 4 : 0)
		| ((switches & SWITCH_S1_RIGHT) ? 2 : 0)
		| ((switches & SWITCH_S2_RIGHT) ? 1 : 0)
		;

	int32_t increment = 0;
	switch( switches_incr ) {
	case 1:  increment = 1;    break;
	case 2:  increment = 10;   break;
	case 3:  increment = 100;  break;
	case 4:  increment = -1;   break;
	case 8:  increment = -10;  break;
	case 12: increment = -100; break;
	}

	if( increment != 0 ) {
		ipc_command_set_frequency(device_state->tuned_hz + (increment * 25000));
	}

	if( switches & SWITCH_S2_UP ) {
		ipc_command_set_if_gain(device_state->if_gain_db + 8);
	}

	if( switches & SWITCH_S2_DOWN ) {
		ipc_command_set_if_gain(device_state->if_gain_db - 8);
	}
}

int main() {
	ipc_init();

	lcd_init();
	lcd_reset();

	lcd_set_background(color_blue);
	lcd_set_foreground(color_white);
	lcd_clear();

	lcd_colors_invert();
	lcd_draw_string(0, 0, "HackRF PortaPack", 16);
	lcd_colors_invert();

	uint32_t frame = 0;
	
	while(1) {
		render_fields(fields, ARRAY_SIZE(fields));

		draw_cycles(0, 128);

		while( lcd_get_scanline() < 200 );
		while( lcd_get_scanline() >= 200 );

		while( !ipc_queue_is_empty() );
		handle_joysticks();

		frame += 1;
		draw_int(frame, "%8d", 256, 0);
	}

	return 0;
}
