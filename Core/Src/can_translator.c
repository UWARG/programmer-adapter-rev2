/*
 * slcan.c
 *
 *  Created on: Nov 1, 2026
 *      Author: arthi
 */

#include <can_translator.h>


#define NUM_OF_BITS_IN_HEX 4U


static CANTranslatorStats_t stats = {0};


void CANTranslator_GetStats(CANTranslatorStats_t* out_stats) {
    *out_stats = stats;
}

uint8_t charToHex(char c) {
	if (c >= '0' && c <= '9') {
		return (c - '0');
	} else if (c >= 'A' && c <= 'F') {
		return ((c - 'A') + 10);
	} else if (c >= 'a' && c <= 'f') {
		return ((c - 'a') + 10);
	}

	return 0;
}

bool SLCANToCAN(CircularBuffer_t* cb, CANQueue_t* canq) {
	static char line_buffer[32];
	static uint8_t index = 0;
	static bool discard_mode = false;

	uint8_t raw_byte;

	uint32_t current_head = cb->head;
	CANMessage_t can_mssg;
	can_mssg.retries = 0;

	while(current_head != cb->tail) {

		if (!CBpop_upto(cb, current_head, &raw_byte)) {
			break; // Reached snapshot limit
		}
		char c = (char)raw_byte;

		if(c == '\r') {
			if (discard_mode) {
				discard_mode = false;
				index = 0;
				continue;
			}

			uint8_t id_length;

			if(line_buffer[0] == 't') {
				id_length = 3;
				can_mssg.is_extended = false;
			} else if (line_buffer[0] == 'T') {
				id_length = 8;
				can_mssg.is_extended = true;
			} else {
				index = 0;
				stats.invalid_command_headers++;
				return false;
			}

			uint32_t id = 0;

			for(int i = 1U; i <= id_length; i++) {
				id = (id << NUM_OF_BITS_IN_HEX) | charToHex(line_buffer[i]);
			}

			can_mssg.id = id;

			uint8_t dlc_index = id_length + 1;
			uint8_t dlc = charToHex(line_buffer[dlc_index]);

			if(dlc > 8) {
				index = 0;
				stats.invalid_dlc_errors++;
				return false;
			}

			can_mssg.dlc = dlc;

			uint8_t payload_start_index = dlc_index + 1;

			for(int i = 0; i < dlc; i++) {
				uint8_t high_nibble = charToHex(line_buffer[payload_start_index + (i * 2)]);
				uint8_t low_nibble = charToHex(line_buffer[payload_start_index + ((i * 2) + 1)]);

				can_mssg.payload[i] = (high_nibble << NUM_OF_BITS_IN_HEX) | low_nibble;
			}

			CANQpush(canq, can_mssg);

			index = 0;

		} else {
			if (discard_mode) {
				continue;
			}

			if (index < sizeof(line_buffer)) {
				line_buffer[index++] = c;
			} else {
				stats.buffer_overflows++;
				discard_mode = true;
			}
		}
	}

	return true;
}

