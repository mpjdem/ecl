#ifndef _ECL_OUTPUT_PFILE_INCLUDED_
#define _ECL_OUTPUT_PFILE_INCLUDED_

#include <string>

#include "ecl_config.h"
#include "ecl_output.h"

using namespace std;

#pragma pack(push,1) //1 byte alignment is necessary for writing the p-files

#define PFILE_TYPE "P-FILE"
#define PFILE_VERSION 202
#define PFILE_OUTPUT_FORMAT 0x2
#define PFILE_NAME_STRING_LENGTH 50
#define PFILE_MON_STRING_LENGTH 16
#define PFILE_N_CAL_POINTS 20
#define PFILE_RESERVED_BYTES 76

// Definition for the STATUS byte
// Bits 0 -> 3 contain the current eye status
#define pFix			0x00		// 0000: fixation
#define pSac			0x01		// 0001: saccade
#define pFixFL			0x02		// 0010: fixation false lock
#define	pSacFL			0x03		// 0011: saccade false lock
#define	pBlink			0x04		// 0100: blink
#define pError			0x05		// 0101: error
#define	pTimeOut		0x06		// 0110: time-out
#define pPuOff			0x07		// 0111: purkinje off
#define	pOsc			0x08		// 1000: oscillation
#define	pNewTrial		0x0F		// 1111: new trial
// Bits 4 -> 7 are reserved for a second tracker 

// Definition for the FLAGS byte
// Bits 0 -> 2 are reserved
#define pStimBit		0x08		// Bit 3: Stimulus on/off
#define pFlag1Bit		0x10		// Bit 4: Flag 1 on/off
#define pFlag2Bit		0x20		// Bit 5: Flag 2 on/off
#define	pRightKeyBit	0x40		// Bit 6: Right key on/off
#define	pLeftKeyBit		0x80		// Bit 7: Left key on/off


//--------------------------------------------------------------------
//  CLASS:			ecl_output_pfile
//	DESCRIPTION:	P-FILE output child class
//--------------------------------------------------------------------
class ecl_output_pfile : public ecl_output
{
public:

															ecl_output_pfile();
															ecl_output_pfile(string fname);
	virtual												  ~ ecl_output_pfile();

	// Base class implementations
			bool											save_buffer(ecl_config * cfg);

	// Own implementations
	//	(none)


protected:

	// Base class implementations
	// (none)

    // Own implementations
	typedef unsigned char BYTE;

	typedef struct
	{
			int											x;
			int											y;
	} POINT;


	typedef struct
	{
			char										id[sizeof(PFILE_TYPE)];			// Type of file
			short int									version;						// Version used (vb. version 3.20 -> 320)
			char										outformat;						// Bit 0 => PU-files (old format), bit 1 => P-files
			char										simulate;        				// TRUE when mouse-simulation was used
			char										subject[PFILE_NAME_STRING_LENGTH];      // Name of subject
			char										experimenter[PFILE_NAME_STRING_LENGTH];	// Name of experimenter
			char										monitor_type[PFILE_N_CAL_POINTS];		// Description of monitor
			short int									w, h;							// Screen range, in pixels
			short int									width, height;					// Screen range, in millimeters
			short int									distance;						// Distance to screen, in millimeters
			short int									pux0, puy0;						// Smallest possible pu-value
			short int									puxrange, puyrange; 			// Range of pu-values (maximum-minimum)
			short int									dx, dy;							// Maximum deviation (in pixels) before FALSE_LOCK
			time_t										ttrial;							// Time of most recent trial start
			time_t										tcalibration;					// Time of most recent calibration
			short int									ncalpoints;						// Number of calibration points
			double										xcor, ycor;						// Correlation between pu and pixel values					
			double										xa, xb;							// Linear equation between pu and pixel X values: x = pux * xa + xb
			double										ya, yb;							// Same for Y
			short int									calx[PFILE_N_CAL_POINTS], caly[PFILE_N_CAL_POINTS];		// Calibration points in pixel values
			short int									calpux[PFILE_N_CAL_POINTS], calpuy[PFILE_N_CAL_POINTS];	// Calibration points in pu values
			short unsigned								maxdt;							// Maximal time between two samples before time-out
			short int									nsleep;							// Starting time in ms (?)
			double										scale_x, scale_y;				// Scaling constants for saccade tracking algorithm
			short int									saccade_lower_bound;			// Minimum value for detecting a saccade
			short int									saccade_upper_bound;			// Maximum value for detecting a saccade
			BYTE										channels;						// Which analog channels are trackeD?
			BYTE										programs;						// Bits indicate which programs edited the data
			double										safe_deviation;					// Safe deviation for fixation
			short int									sac_safe_velocity;				// Minimum velocity for a proper saccade
			short int									sac_safe_duration;				// Minimum duration for a proper saccade
			BYTE										reserved[PFILE_RESERVED_BYTES];	// Should be set to zero!
	} pfile_cfg;

			pfile_cfg									m_pfile_cfg;
};

#pragma pack(pop)	// Back to 8-byte alignment

#endif	//_ECL_OUTPUT_PFILE_INCLUDED_