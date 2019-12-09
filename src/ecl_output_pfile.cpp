#include "stdafx.h"

#include <vector>
#include <string>

#include "ecl.h"
#include "ecl_output_pfile.h"

using namespace std;


//--------------------------------------------------------------------
//	FUNCTION:		ecl_output_pfile::ecl_output_pfile()
//	DESCRIPTION:	Constructor
//--------------------------------------------------------------------
ecl_output_pfile::ecl_output_pfile()
{
	m_fname = "";
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_output_pfile::ecl_output_pfile()
//	DESCRIPTION:	Constructor
//--------------------------------------------------------------------
ecl_output_pfile::ecl_output_pfile(string fname)
{

}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_output_pfile::~ecl_output_pfile()
//	DESCRIPTION:	Deconstructor
//--------------------------------------------------------------------
ecl_output_pfile::~ecl_output_pfile()
{

}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_output_pfile::save_buffer(ecl_config * cfg)
//	DESCRIPTION:	Save buffer to p-file
//--------------------------------------------------------------------
bool ecl_output_pfile::save_buffer(ecl_config * cfg)
{

	/*------------------------------------------------------------------
						*****************
						***SAVE P-FILE***
						*****************

		Small explanation:
	- First, the m_cfg struct is copied from memory to the file. Since pview.exe
		assumes there is no padding in the file, m_cfg has to be 1-byte aligned, or
		pview.exe will not be able to read its contents.
	- Then the comments for the first trial are saved. First, a placeholder long
		is written. Then, based on the timing of the comment, 1-4 bytes are allocated to
		writing down this time difference. Then the first comment is written. A 00 byte
		signals the end of the comment. Then the second comment could be written, again
		preceded by 1-4 bytes of timedifference. When all comments are written, an extra 00
		byte is written to signal the end of all comments. The program then goes back and 
		looks how large the comment block is in total, and writes the size of the comment
		block to the reserved long right before the comment block.
	- Then data block is written. Again, it is preceded by a reserved long that will later
		contain the size of the block. Another long follows, containing the begin time of the
		trial. Then the actual samples are written. Again, it is computed what the timedifference
		is and how much space this is going to take, then the timedifference (duration of the previous
		state) is written. Everytime something changes, this is written again, along with what has changed.
		When all samples are written, a 00 byte finalizes the trial. The total size of the data block is then
		written to the long preceding the data block.
	------------------------------------------------------------------*/

	// Check for errors
	if(!m_b_buffer_allocated)
	{	
		ecl_error("ECL: ecl_output_pfile::save_buffer - No buffer exists");
        return false;
	}

	if(m_fname == "")
	{	
		ecl_error("ECL: ecl_output::save_buffer - No valid filename specified");
        return false;
	}

	if(m_n_samples < 1)
	{	
		ecl_error("ECL: ecl_output_pfile::save_buffer - Buffer is empty");
        return false;
	}

	if(cfg == NULL)
	{	
		ecl_error("ECL: ecl_output_pfile::save_buffer - No configuration supplied");
        return false;
	}

	if(m_subject_name.size() >= PFILE_NAME_STRING_LENGTH || m_experimenter_name.size() >= PFILE_NAME_STRING_LENGTH || m_monitor_name.size() >= PFILE_MON_STRING_LENGTH)
	{	
		ecl_error("ECL: ecl_output_pfile::save_buffer - Name strings are too long");
        return false;
	}

	if(!m_monitor_h_px || !m_monitor_w_px || !m_monitor_h_mm || !m_monitor_w_mm || !m_monitor_distance)
	{	
		ecl_error("ECL: ecl_output_pfile::save_buffer - Missing necessary monitor information");
        return false;
	}


	// Fill in the p-file style configuration
	memset((void *)&m_pfile_cfg, 0, sizeof(m_pfile_cfg));

	strcpy(m_pfile_cfg.id,PFILE_TYPE);
	m_pfile_cfg.version=PFILE_VERSION;							
	m_pfile_cfg.outformat=PFILE_OUTPUT_FORMAT;
	m_pfile_cfg.simulate=0; // Too much hassle, leave it at 0
	strcpy(m_pfile_cfg.subject, m_subject_name.c_str());			
	strcpy(m_pfile_cfg.experimenter, m_experimenter_name.c_str());	
	strcpy(m_pfile_cfg.monitor_type, m_monitor_name.c_str());

	m_pfile_cfg.w = (short int) m_monitor_w_px;		
	m_pfile_cfg.h = (short int) m_monitor_h_px;
	m_pfile_cfg.width = (short int) m_monitor_w_mm;
	m_pfile_cfg.height = (short int) m_monitor_h_mm;
	m_pfile_cfg.distance = (short int) m_monitor_distance;		

	m_pfile_cfg.ncalpoints = (short int)cfg->m_cal_points_n;	

	long min_pix_x = m_monitor_w_px;
	long min_pix_y = m_monitor_h_px;
	long max_pix_x = 0;
	long max_pix_y = 0;

	for (int i=0;i<m_pfile_cfg.ncalpoints;i++)
	{
		m_pfile_cfg.calx[i]= (short int)cfg->m_cal_pixvals_x[i];
		m_pfile_cfg.caly[i]= (short int)cfg->m_cal_pixvals_y[i];
		m_pfile_cfg.calpux[i]= (short int)cfg->m_cal_daqvals_x[i];
		m_pfile_cfg.calpuy[i]= (short int)cfg->m_cal_daqvals_y[i];

		if (cfg->m_cal_pixvals_x[i] < min_pix_x)
			{min_pix_x = cfg->m_cal_pixvals_x[i];}
		if (cfg->m_cal_pixvals_y[i] < min_pix_y)
			{min_pix_y = cfg->m_cal_pixvals_y[i];}
		if (cfg->m_cal_pixvals_x[i] > max_pix_x)
			{max_pix_x = cfg->m_cal_pixvals_x[i];}
		if (cfg->m_cal_pixvals_y[i] > max_pix_y)
			{max_pix_y = cfg->m_cal_pixvals_y[i];}
	}

	m_pfile_cfg.xa = cfg->m_cal_params[0];						
	m_pfile_cfg.xb = cfg->m_cal_params[1];														
	m_pfile_cfg.ya = cfg->m_cal_params[2];															
	m_pfile_cfg.yb = cfg->m_cal_params[3];	

	m_pfile_cfg.pux0 = (short int)(((double)min_pix_x - cfg->m_cal_params[1])/cfg->m_cal_params[0]);
	m_pfile_cfg.puy0 = (short int)(((double)min_pix_y - cfg->m_cal_params[3])/cfg->m_cal_params[2]);	
	m_pfile_cfg.puxrange = (short int)(((double)(max_pix_x-min_pix_x) - cfg->m_cal_params[1])/cfg->m_cal_params[0]);
	m_pfile_cfg.puyrange = (short int)(((double)(max_pix_y-min_pix_y) - cfg->m_cal_params[3])/cfg->m_cal_params[2]);
	m_pfile_cfg.dx = (short int)floor(0.5+(tan((cfg->m_cal_flocktol_deg_x*PI)/180)*m_monitor_distance));
	m_pfile_cfg.dy = (short int)floor(0.5+(tan((cfg->m_cal_flocktol_deg_y*PI)/180)*m_monitor_distance));

	m_pfile_cfg.ttrial = m_first_sample_time;
	m_pfile_cfg.tcalibration = cfg->m_cal_time;
	m_pfile_cfg.xcor = cfg->m_cal_fit[0];					
	m_pfile_cfg.ycor = cfg->m_cal_fit[1];	

	m_pfile_cfg.maxdt = (short unsigned)cfg->m_timeout_limit;			
	m_pfile_cfg.nsleep = 0;				

	m_pfile_cfg.scale_x = ((double)cfg->m_sac_scale_nominator) / (((double)m_pfile_cfg.puxrange)/cfg->m_cal_range_deg_x);
	m_pfile_cfg.scale_y = ((double)cfg->m_sac_scale_nominator) / (((double)m_pfile_cfg.puyrange)/cfg->m_cal_range_deg_y);	
	m_pfile_cfg.saccade_lower_bound = (short int)cfg->m_sac_lower_bound;	
	m_pfile_cfg.saccade_upper_bound = (short int)cfg->m_sac_upper_bound;	
	m_pfile_cfg.channels = 0x0F;								
	m_pfile_cfg.programs = 0x80;		

	m_pfile_cfg.safe_deviation = cfg->m_fix_safe_deg;			
	m_pfile_cfg.sac_safe_velocity =(short int) cfg->m_sac_safe_velocity;	
	m_pfile_cfg.sac_safe_duration = (short int) cfg->m_sac_safe_duration;	
	
	for (int i=0;i< PFILE_RESERVED_BYTES;i++)
	{
		m_pfile_cfg.reserved[i] = 0;
	}

	// Write config if the file is new
	FILE *f;
	f = fopen(m_fname.c_str(), "rb+");
	if (f == NULL)
	{
		f = fopen(m_fname.c_str(), "wb+");
		
		if (f) 
		{
			int q = sizeof(pfile_cfg);
			fwrite(&m_pfile_cfg, q, 1, f);
		}
	}

	if (f == NULL)
	{	
		ecl_error("ECL: ecl_output_pfile::save_buffer - Could not open file");
        return false;
	}


	// Write comments
	long blocksize = 1;
	long dt;
	BYTE codebyte;
	fseek(f, 0, SEEK_END);								// To the end of the file (else ftell does not work)
	long comment_pos = ftell(f);								
	fwrite(&blocksize, sizeof(long), 1, f);				// Write comment block size; at this moment 1 byte; the terminating 0
		
	if (m_commentbuf.size() > 0)						// If there are any comments
	{
		unsigned long tcomment, tprev;
		tprev = 0;
	
		std::vector<commentstruct>::iterator it = m_commentbuf.begin();		// Iterate over all comments
		while(it < m_commentbuf.end())
		{
			if (it->tstamp >= m_samplebuf[0].tstamp)
				{tcomment =	it->tstamp - m_samplebuf[0].tstamp;}			// Fetch the time of this comment
			else {tcomment = 0;}

			codebyte = 0x80;												// Assume everything is fine	
			dt = tcomment - tprev;											// Calculate dimediff		
			tprev = tcomment;				

			/*	Based on timediff, write 
					1) How many bytes the timediff will take to write
					2) The timediff itself
		
				General format: the first bit signals there was no error (1) or an error (0)
				The next two bits signal how many bytes are reserved for containing the timediff
					1) 01 means 1 byte only (5 bits after the initial 3)
					2) 10 means 2 bytes (13 bits after the initial 3)
					3) 11 means more than 2 bytes
				Starting from the 4th bit, the actual timediff is written
			*/
		
			if (dt <= 0x1F)						// We'll need just one byte
			{
				codebyte |= (BYTE)dt | 0x20;
				fputc(codebyte, f);

				/*
					In this case, dt is minimally 0000 0000 in bits (00 in hex, 0 in dec) 
			              and maximally 0001 1111 in bits (1F in hex, 31 in dec). 
					0x20 in hex equals 0010 0000 in bits
					Codebyte equals 0x80 if no error, i.e. 1000 0000 and 0x00 if error, i.e. 0000 0000
					The bits of codebyte, (BYTE)dt and 0x20 will be combined through an OR operation
					I.e., if one or more of these bytes has a 1 in a certain bit location, the result will also have a 1 there
					The result is stored in codebyte, and written to the file.

					As an example: there is no error and dt = 27
					dt			= 0001 1011
					0x20		= 0010 0000
					codebyte	= 1000 0000
					=> codebyte	= 1011 1011
				*/
			}
			else if (dt <= 0x1FFF)				// We'll need 2 bytes
			{				
				codebyte |= (BYTE)(dt >> 8) | 0x40;
				fputc(codebyte, f); 
				fputc((BYTE)dt, f);

				/* 
					Here, dt is minimally 0000 0000 0010 0000 (0020 in hex, 32 in dec) 
						and maximally 0001 1111 1111 1111 (1FFF in hex, 8191 in dec)
					The >> 8 moves all bits 8 positions, removing the 2^0 to 2^7 bits of these 2 bytes
					So, what remains ranges from 00 to 1F again, as in the above case
					0x40 in hex equals 0100 0000 in bits
					Codebyte is again either 1000 0000 or 0000 0000
					The result is then combined, exactly as above, into codebyte.
					Only, this time the 2nd and 3rd bits being 10 signals that there is a byte to follow still
					So first this codebyte is written, 
					then (BYTE)dt fetches the 2^0 to 2^7 bits of the original dt again and they are written also
			
					Again an example, where dt = 548 and there is no error
					dt			= 0000 0010 0010 0100
					(BYTE)dt>>8	= 0000 0010
					0x40		= 0100 0000
					codebyte	= 1000 0000
					=> codebyte	= 1100 0010
	
					(BYTE)dt	= 0010 0100	

					=> Written: 1100 0010 0010 0100
				*/

			}
			else								// We'll need more than 2 bytes
			{
				codebyte |= (BYTE)(tcomment >> 24) | 0x60;
				fputc(codebyte, f);
				fwrite(&tcomment, 3, 1, f);

				/* 

				Here, dt is minimally 0010 0000 0000 0000 0000 0000 0000 0000 (2000 0000 in hex, 8192 in dec)
					   and maximally 1111 1111 1111 1111 1111 1111 1111 1111 (FFFF FFFF in hex, 4294967295 in dec: max of a 4-byte unsigned long)
				However, not dt but tcomment itself is written, also a 4 byte unsigned long
				Tcomment is shifted 24 bits (3 bytes) here, only the leftmost byte is retained 
				0x60 in hex equals 0110 0000 in bits
				Codebyte is again either 1000 0000 or 0000 0000
				Again these three bytes are combined through an OR operation, like above
				The resulting codebyte is written, followed by the remaining 3 bytes of tcomment
				
				This part of code seems to assume that really large values never occur or shouldn't be accurate, 
				as it basically overwrites the first three bits of tcomment.				
				*/
			}

				fputs(it->comment_text.c_str(), f);				// The comment itself is written
				fputc(0, f);									// A 00 signals the end of this comment
				it++;											// On to the next comment
		}
	}

		fputc(0, f);											// Comment block end

		long data_pos = ftell(f);								// Remember where the samples begin
		blocksize = data_pos - comment_pos - sizeof(long);		
		fseek(f, comment_pos, SEEK_SET);
		fwrite(&blocksize, sizeof(long), 1, f);					// Write size of comment block at the beginning
		fseek(f, data_pos, SEEK_SET);
		blocksize = 1;
		fwrite(&blocksize, sizeof(long), 1, f);					// Write size of data block (for now)

		long tsleepstart = 0;
		fwrite(&tsleepstart, sizeof(long), 1, f);				// Write begin time


	// Write samples
	if (m_n_samples)
	{
		BYTE		next_status=pPuOff, previous_status=pPuOff;
		BYTE		next_flags=0, previous_flags=0;
		POINT		next_xy = {m_samplebuf[0].daq_x, m_samplebuf[0].daq_y}, prev_xy = {0,0};
		long		sample_no, tsample, tstart;
		int			dx, dy;

		long nsamples = m_n_samples;
		tsample = 0;
		sample_no = 0;
		previous_status = ~next_status;							// Definitely write the first status
		codebyte = 0;

		/* Small explanation. The program will simply go through the samples buffer, and find
			any changes. When it finds them, it will write these changes and the duration of the previous
			state. A codebyte is used to signal to pview.exe what changes have occurred.
			This codebyte is written first. Then, the duration of the previous state is written
			if it exceeded 1ms; the size needed to write the duration is again coded with it,
			so that it can change dynamically (see save_comments for a detailed explanation).
			Then changes on the X dimension, changes on the Y dimension, changes in status and
			changes in flags are written, but only if they occurred. Then the program continues
			with the next state until all samples have been processed.
		*/

		do
		{
			codebyte = 0;			// 0000 0000

			// Update codebyte: which changes occurred at tstart = tsample ?
			tstart = tsample;
			dx = next_xy.x - prev_xy.x;
			dy = next_xy.y - prev_xy.y;
		
			if (dx)		// If a change occurred on the X dimension
			{ 
				codebyte |= 1;		// 0000 0001
			}
			if (dy)		// If a change occurred on the Y dimension
			{ 
				codebyte |= 2;		// 0000 0010
			}
		
			if (next_status != previous_status)		// If status changed
			{
				codebyte |= 0x10;	// 0001 0000
			}
			if (next_flags != previous_flags)		// If flags change
			{	
				codebyte |= 0x20;	// 0010 0000
			}
			prev_xy = next_xy;
			previous_status = next_status;
			previous_flags = next_flags;

			// Find beginning of next (status,x,y) state, or end of trial
			// Basically, a while loop until something is changed
			do
			{
				tsample++; 
				sample_no = tsample;
				if (sample_no >= (long)nsamples)
				{
					if (previous_status == pPuOff) 
						{return true;}	// End of trial, do not save pause
					else 
					{	
						break;			// End of trial, save last state
					}
				}
				next_xy.x = m_samplebuf[sample_no].daq_x;
				next_xy.y = m_samplebuf[sample_no].daq_y;

				// Convert ECL status bytes to pfile status byte
				next_status = 0x00; // start from good safe fixation (0000)

				if(m_samplebuf[sample_no].status &= 0x01)			// if saccade
					{next_status |= 1 << 1;} // both on bit 1

				if(!(m_samplebuf[sample_no].status &= 0x02))		// if not safe
					{next_status |= 1 << 4;} // Bit 2 to -(bit 4)

				if(!(m_samplebuf[sample_no].status &= 0x04))		// if false lock
					{next_status |= 1 << 2;} // Bit 3 to -(bit 2)

				if(m_samplebuf[sample_no].status &= 0x08)			// if blink
					{next_status = pBlink;}

				if(!(m_samplebuf[sample_no].status &= 0x10))		// if no track
					{next_status = pError;}

				if(!(m_samplebuf[sample_no].status &= 0x20))		// if timeout
					{next_status = pTimeOut;}

				next_flags = 0x00; // all off

				if(m_samplebuf[sample_no].flags_stim &= 0x01)		// If first stim
					{next_flags |= 1 << 3;}	// Bit 3

				if(m_samplebuf[sample_no].flags_other &= 0x01)		// If Flag 1
					{next_flags |= 1 << 4;}	// Bit 4

				if(m_samplebuf[sample_no].flags_other &= 0x02)		// If Flag 2
					{next_flags |= 1 << 5;}	// Bit 5

				if(m_samplebuf[sample_no].flags_keys &= 0x01)		// If Key 1 (right)
					{next_flags |= 1 << 6;}	// Bit 6

				if(m_samplebuf[sample_no].flags_keys &= 0x02)		// If Key 2 (left)
					{next_flags |= 1 << 7;}	// Bit 7

			} while (previous_status == next_status && previous_flags == next_flags && next_xy.x == prev_xy.x && next_xy.y == prev_xy.y);


			// New (x,y,status,flags) state, determine how long the previous state lasted, and save it
			// dt+2 is the duration of the previous (x,y,status,flags) state
			// tstart is the beginning of the previous (x,y,status,flags), tsample-1 is the end
			
			if (tstart != 0)
			{
				dt = (m_samplebuf[tsample-1].tstamp) - (m_samplebuf[tstart].tstamp) - 2 + (m_samplebuf[tstart].tstamp - m_samplebuf[tstart-1].tstamp);
			// Last part of the formula is necessary for including the length of missing samples
			}
			else
			{
				dt = (m_samplebuf[tsample-1].tstamp) - (m_samplebuf[tstart].tstamp) - 1;
			}
			// To avoid an error with the pointer; the first state can't be 'timeout' anyway.

			if (dt >= 0)	
			// If dt < 0 -> dt = -1 -> duration = 1 ms -> no timecode needed
			// Has the same effect as codebyte |= 0000 0000
			{
				if (dt <= 0xFFL)		// 1 byte necessary
					codebyte |= 0x40;		// 0100 0000
				else if	(dt <= 0xFFFFL)	// 2 bytes necessary
					codebyte |= 0x80;		// 1000 0000
				else					// More than two bytes necessary
					codebyte |= 0xC0;		// 1100 0000
			}


			// Save previous state, and its duration
			fputc(codebyte, f);			// Write codebyte, contains info on what exactly is saved
			if (codebyte & 0xC0)		// If time code is needed (a 1 bit in any of the first two bits)
			{
				if ((codebyte & 0xC0) == 0xC0)	// More than 2 bytes are necessary
					fwrite(&dt, sizeof(long), 1, f);		// Write timediff in 4 bytes
				else if (codebyte & 0x80)		// 2 bytes necessary
					fwrite(&dt, sizeof(short int), 1, f);	// Write in 2 bytes
				else							// 1 byte necessary
					fputc((BYTE)dt, f);						// Write as one byte
			}

			if (codebyte & 1)			// If a change on the X-dimension occurred
			{
				// Part that is commented out does not seem necessary except for saving disk space
				// and it makes things more complicated / causes problems
				// Therefore we just write the position itself instead of the position difference
				/*if (dx > 0xF && dx <= 0xFF)		// If one byte is enough (what happens if dx<0xF?? Nothing?? Oscillation??)
				{
					fputc(dx, f);				// Write one byte
				}
				else							// If not (thus, 2 bytes - I think x and y can in practice never be larger than FFFF)
				{*/	
					fputc(prev_xy.x >> 8, f);	// Write first byte
					fputc(prev_xy.x, f);		// Write second byte
				//} 
			}

			if (codebyte & 2)			// If a change on the Y-dimension occurred
			{
				/*if (dy > 0xF && dy <= 0xFF)		
				{	
					fputc(dy, f);				
				}
				else							
				{ */
					fputc(prev_xy.y >> 8, f);	
					fputc(prev_xy.y, f);	
				//}
			}

			if (codebyte & 0x10)		// If a change in status occurred
				fwrite(&previous_status, sizeof(BYTE), 1, f);	// Write it in one byte

			if (codebyte & 0x20)		// If a change in flags occurred
				fwrite(&previous_flags, sizeof(BYTE), 1, f);	// Write it in one byte
		} while (sample_no <(long) nsamples);

		fputc(0, f);										// Trial end
		comment_pos = ftell(f);								// Position of the next comment block
		blocksize = comment_pos - data_pos - sizeof(long);	// Size of data block
		fseek(f, data_pos, SEEK_SET);
		fwrite(&blocksize, sizeof(long), 1, f);				// Write size of data block at the beginning of the block
		
	}	
		else 
		{return false;}

	fclose(f);
	return true;
}
//