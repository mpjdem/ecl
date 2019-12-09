#ifndef _ECL_H_INCLUDED_
#define _ECL_H_INCLUDED_

#pragma warning(disable: 4786)

#include <vector>
#include <string>

using namespace std;

#define Sqr(i)										((i)*(i))
#define null										0x0000
#define PI											3.14159265

#define ECL_SAMPLE_FREQ                             1000
#define ECL_ERROR_BUFFER_SIZE						100
#define	ECL_ERROR_BUFFER_NAME						"ecl_errors.txt"

// Init & cleanup	
bool												ecl_init();
bool												ecl_cleanup();

// ECL timer
long												ecl_get_time();
long												ecl_reset_timer();			// Perhaps this function should be protected, in case a DAQ is using the timer
void												ecl_wait_time(long t);

// LUT color system
typedef int ecl_LUTc;
ecl_LUTc											ecl_make_LUTcolor(int r, int g, int b);

// Error system
bool												ecl_die(string err_msg);	// Throw exception, stop program
bool												ecl_error(string err_msg);	// Dump string to message buffer and continue
bool												ecl_dump_error_buf();
extern vector<string>								ecl_error_buf;		


#endif	//_ECL_H_INCLUDED_