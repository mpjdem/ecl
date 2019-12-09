#include "stdafx.h"

#include "ecl.h"
#include "ecl_output.h"

using namespace std;


//--------------------------------------------------------------------
//	FUNCTION:		ecl_output::ecl_output()
//	DESCRIPTION:	Constructor
//--------------------------------------------------------------------
ecl_output::ecl_output()
{
	m_fname = "";
	m_b_buffer_allocated = false;
	m_bufsize = 0;
	m_n_samples = 0;

	m_daq_source = "DAQ";
	m_experimenter_name = "ME";
	m_subject_name = "YOU";
	m_session_id = "NONE";
	m_monitor_name = "MONITOR";
	m_monitor_h_px = 0;
	m_monitor_w_px = 0;
	m_monitor_h_mm = 0;
	m_monitor_w_mm = 0;
	m_monitor_distance = 0;
	m_monitor_framerate = 0;
	m_monitor_bitdepth = 0;
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_output::~ecl_output()
//	DESCRIPTION:	Deconstructor
//--------------------------------------------------------------------
ecl_output::~ecl_output()
{
	destroy_buffer();
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_output::create_buffer()
//	DESCRIPTION:	Allocate memory for one trial
//--------------------------------------------------------------------
bool ecl_output::create_buffer(long bufsize)
{
	if(m_b_buffer_allocated)
	{	
		ecl_error("ECL: ecl_output::create_buffer - A buffer already exists");
        return false;
	}

	if(m_fname == "")
	{	
		ecl_error("ECL: ecl_output::create_buffer - No valid filename specified");
        return false;
	}

	m_n_samples = 0;
	m_bufsize = bufsize;

	try
		{m_samplebuf.reserve(m_bufsize);}
	catch(...)
	{
		ecl_error("ECL: ecl_output::create_buffer - Not enough memory");
        return false;
	}

	try
		{m_commentbuf.reserve(ECL_OUTPUT_COMMENT_BUFFER_SIZE);}
	catch(...)
	{
		ecl_error("ECL: ecl_output::create_buffer - Not enough memory");
        return false;
	}

	m_b_buffer_allocated = true;

	return true;
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_output::clear_buffer()
//	DESCRIPTION:	Keep the memory allocated, but remove all information
//--------------------------------------------------------------------
bool ecl_output::clear_buffer()
{
	if(!m_b_buffer_allocated)
	{	
		ecl_error("ECL: ecl_output::clear_buffer - No buffer exists");
        return false;
	}

	m_samplebuf.clear();
	m_commentbuf.clear();

	return true;
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_output::destroy_buffer()
//	DESCRIPTION:	Clear buffer, remove memory allocation
//--------------------------------------------------------------------
bool ecl_output::destroy_buffer()
{
	if(!m_b_buffer_allocated)
	{	
		ecl_error("ECL: ecl_output::destroy_buffer - No buffer exists");
        return false;
	}

	m_samplebuf.clear();
	m_commentbuf.clear();

	vector<samplestruct>().swap(m_samplebuf);
	vector<commentstruct>().swap(m_commentbuf);

	m_n_samples = 0;
	m_bufsize = 0;
	m_b_buffer_allocated = false;
	return true;
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_output::add_sample()
//	DESCRIPTION:	Add a sample to the buffer
//--------------------------------------------------------------------
bool ecl_output::add_sample(long tstamp, long daq_x, long daq_y, long pix_x, long pix_y, vector<bool> status, vector<bool> stims, vector<bool> keys, vector<bool> flags)
{
	if (m_samplebuf.size() >= m_bufsize)
	{
		ecl_error("ECL: ecl_output::add_sample - Too many samples");
        return false;
	}
	
	samplestruct sample;

	// Time stamp
	sample.tstamp = tstamp;

	// Daq and pix coordinates
	if (abs(daq_x) > 32000 || abs(daq_y) > 32000 || abs(pix_x) > 32000 || abs(pix_y) > 32000)
	{
		ecl_error("ECL: ecl_output::add_sample - Coordinates exceed maximum range");
        return false;
	}	

	sample.daq_x = (short int) daq_x;
	sample.daq_y = (short int) daq_y;
	sample.pix_x = (short int) pix_x;
	sample.pix_y = (short int) pix_y;

	int n;

	// Status
	if (status.size() > 8)
	{
		ecl_error("ECL: ecl_output::add_sample - Maximum 8 status booleans");
        return false;
	}	

	n = 0;
	for (vector<bool>::iterator it = status.begin(); it!=status.end(); ++it) 
	{
		if(*it)
			{sample.status |= 1 << n;}
		n++;
	}

	// Stim flags
	if (stims.size() > 16)
	{
		ecl_error("ECL: ecl_output::add_sample - Maximum 16 stimulus flags");
        return false;
	}

	sample.flags_stim = 0;
	n = 0;
	for (vector<bool>::iterator it = stims.begin(); it!=stims.end(); ++it) 
	{
		if(*it)
			{sample.flags_stim |= 1 << n;}
		n++;
	}

	// Key flags
	if (keys.size() > 16)
	{
		ecl_error("ECL: ecl_output::add_sample - Maximum 16 key flags");
        return false;
	}

	sample.flags_keys = 0;
	n = 0;
	for (vector<bool>::iterator it = keys.begin(); it!=keys.end(); ++it) 
	{
		if(*it)
			{sample.flags_keys |= 1 << n;}
		n++;
	}

	// Other flags
	if (flags.size() > 16)
	{
		ecl_error("ECL: ecl_output::add_sample - Maximum 16 other flags");
        return false;
	}

	sample.flags_other = 0;
	n = 0;
	for (vector<bool>::iterator it = flags.begin(); it!=flags.end(); ++it) 
	{
		if(*it)
			{sample.flags_other |= 1 << n;}
		n++;
	}

	// Save to buffer

	if(!m_n_samples)
	{
		time(&m_first_sample_time);
	}

	m_samplebuf.push_back(sample);
	m_n_samples++;

	return true;
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_output::add_comment()
//	DESCRIPTION:	Add a comment to the buffer
//--------------------------------------------------------------------
bool ecl_output::add_comment(long tstamp, string comment_text)
{
	if (m_commentbuf.size() >= ECL_OUTPUT_COMMENT_BUFFER_SIZE)
	{
		ecl_error("ECL: ecl_output::add_comment - Too many comments");
        return false;
	}
	
	commentstruct comment;
	comment.tstamp = tstamp;
	comment.comment_text = comment_text;

	m_commentbuf.push_back(comment);

	return true;
}
//