/*
 * error_reporter.h
 *
 *  Created on: Nov 9, 2026
 *      Author: Tejas
 */

/*
This is just a skeleton
Only improtant thing is calling report_error
or report_warning. its an error if the program should abort
gotta clean up the rest of the internal working still
*/

/*
Error code definitions and descriptions are in error_codes.h.
*/

#ifndef INC_ERROR_REPORTER_H_
#define INC_ERROR_REPORTER_H_

// One shared store is owned by error_reporter.c and starts zeroed at boot.
// Call these functions from main or maskable interrupt handlers; no object or
// initialization call is needed. NMI and HardFault handlers are not supported.

// Record a fatal error and halt; this function does not return.
void report_error(int error_code);

// Record a nonzero warning code and return, preserving the interrupt mask.
// Stores up to 16 warnings; additional warnings are discarded.
void report_warning(int warning_code);

#endif /* INC_ERROR_REPORTER_H_ */
