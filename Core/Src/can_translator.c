/*
 * slcan.c
 *
 *  Created on: Nov 1, 2026
 *      Author: arthi
 */

#include <can_translator.h>


#define NUM_OF_BITS_IN_HEX 		4U
#define HEX_DIGIT_MASK 			0x0F


static CANTranslatorStats_t stats = {0};


void CANTranslator_GetStats(CANTranslatorStats_t* out_stats) {
    *out_stats = stats;
}

bool charToHex(char c, uint8_t* out_val) {
	if (c >= '0' && c <= '9') {
		*out_val = (uint8_t)(c - '0');
		return true;
	} else if (c >= 'A' && c <= 'F') {
		*out_val = (uint8_t)((c - 'A') + 10);
		return true;
	} else if (c >= 'a' && c <= 'f') {
		*out_val = (uint8_t)((c - 'a') + 10);
		return true;
	}

	return false;
}

bool hextoChar(uint8_t hex, char* out_val) {
	if (hex > 0xF) {
		return false;
	}

	*out_val = hextoCharLUT[hex];
	return true;
}

bool SLCANToCAN(CircularBuffer_t* cb, CANQueue_t* canq) {
	uint32_t current_head = cb->head;

	static char line_buffer[32];
	static uint8_t index = 0;
	static bool discard_mode = false;

	uint8_t raw_byte;

	CANMessage_t can_mssg;
	can_mssg.retries = 0;

	while(CBpop_upto(cb, current_head, &raw_byte)) {

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

			uint8_t hex_digit;

			for(int i = 1U; i <= id_length; i++) {
				if(!charToHex(line_buffer[i], &hex_digit)) {
					stats.invalid_command_headers++;
					goto parse_error;
				}
				id = (id << NUM_OF_BITS_IN_HEX) | hex_digit;
			}

			can_mssg.id = id;

			uint8_t dlc_index = id_length + 1;

			if(!charToHex(line_buffer[dlc_index], &hex_digit)) {
				stats.invalid_command_headers++;
				goto parse_error;
			}

			uint8_t dlc = hex_digit;

			if(dlc > 8) {
				index = 0;
				stats.invalid_dlc_errors++;
				return false;
			}

			can_mssg.dlc = dlc;

			uint8_t payload_start_index = dlc_index + 1;

			for(int i = 0; i < dlc; i++) {
				if(!charToHex(line_buffer[payload_start_index + (i * 2)], &hex_digit)) {
					stats.invalid_command_headers++;
					goto parse_error;
				}

				uint8_t high_nibble = hex_digit;

				if(!charToHex(line_buffer[payload_start_index + ((i * 2) + 1)], &hex_digit)) {
					stats.invalid_command_headers++;
					goto parse_error;
				}

				uint8_t low_nibble = hex_digit;

				can_mssg.payload[i] = (high_nibble << NUM_OF_BITS_IN_HEX) | low_nibble;
			}

			if(!CANQpush(canq, can_mssg)) {
				stats.buffer_overflows++;
				return false;
			}

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

	parse_error:
	    index = 0;
	    return false;
}

bool CANToSLCAN(CircularBuffer_t* cb, CANQueue_t* canq) {
    uint32_t current_head = canq->head;
    CANMessage_t raw_can_mssg;

    while (CANQpop_upto(canq, current_head, &raw_can_mssg)) {
        char ascii_digit;
        uint8_t id_length;

        if (raw_can_mssg.is_extended) {
        	if (!CBpush(cb, 'T')) {
        		stats.buffer_overflows++;
        		return false;
        	}
            id_length = 8;
        } else {
        	if (!CBpush(cb, 't')) {
        		stats.buffer_overflows++;
        		return false;
        	}
            id_length = 3;
        }

        for (int i = id_length - 1; i >= 0; i--) {
            uint8_t id_digit = (uint8_t)((raw_can_mssg.id >> (i * NUM_OF_BITS_IN_HEX)) & HEX_DIGIT_MASK);

            if (!hexToChar(id_digit, &ascii_digit)) {
                stats.invalid_command_headers++;
                return false;
            }
            if (!CBpush((cb, (uint8_t)ascii_digit))) {
				stats.buffer_overflows++;
				return false;
			}
        }

        if (raw_can_mssg.dlc > 8) {
            stats.invalid_dlc_errors++;
            return false;
        }
        if (!hexToChar(raw_can_mssg.dlc, &ascii_digit)) {
            stats.invalid_command_headers++;
            return false;
        }
        if (!CBpush((cb, (uint8_t)ascii_digit))) {
			stats.buffer_overflows++;
			return false;
		}

        for (int i = 0; i < raw_can_mssg.dlc; i++) {
            uint8_t high_nibble = (raw_can_mssg.payload[i] >> NUM_OF_BITS_IN_HEX) & HEX_DIGIT_MASK;
            uint8_t low_nibble  = raw_can_mssg.payload[i] & HEX_DIGIT_MASK;

            if (!hexToChar(high_nibble, &ascii_digit)) {
                stats.invalid_command_headers++;
                return false;
            }
            if (!CBpush((cb, (uint8_t)ascii_digit))) {
    			stats.buffer_overflows++;
    			return false;
    		}
            if (!hexToChar(low_nibble, &ascii_digit)) {
                stats.invalid_command_headers++;
                return false;
            }
            if (!CBpush((cb, (uint8_t)ascii_digit))) {
    			stats.buffer_overflows++;
    			return false;
    		}
        }

        if (!CBpush((cb, '\r'))) {
			stats.buffer_overflows++;
			return false;
		}
    }

    return true;
}
