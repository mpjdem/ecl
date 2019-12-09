#include "stdafx.h"

#include "ecl.h"
#include "ecl_daq.h"

using namespace std;


//--------------------------------------------------------------------
//	FUNCTION:		ecl_daq::ecl_daq()
//	DESCRIPTION:	Constructor
//--------------------------------------------------------------------
ecl_daq::ecl_daq()
{
    m_nchan = 0;
    m_freq = ECL_SAMPLE_FREQ;
    m_b_recording = false;
    m_b_initialised = false;
    m_b_sampling = false;
    m_rec_bufsize = 0;
    m_rec_file = NULL;
    m_rec_fname = "";
	m_rec_pos = 0;
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_daq::~ecl_daq()
//	DESCRIPTION:	Deconstructor
//--------------------------------------------------------------------
ecl_daq::~ecl_daq()
{
    stop_recording();

	m_rec_channel_log.clear();
	m_rec_tstamp_log.clear();

	vector<long>().swap(m_rec_channel_log);
	vector<long>().swap(m_rec_tstamp_log);

	m_rec_file = NULL;
	m_rec_pos = 0;
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_daq::init()
//	DESCRIPTION:	Initialize base & child class functionality
//--------------------------------------------------------------------
bool ecl_daq::init(long nchan, long rec_bufsize)
{
    if (m_b_initialised)
        {return true;}

    if (nchan > 0)
    {
        m_nchan = nchan;
    }
	else if (nchan > 10)
	{
        ecl_error("ECL: ecl_daq::init - Maximally 10 DAQ channels are allowed");
        return false;
	}
    else
    {
        ecl_error("ECL: ecl_daq::init - Need at least one DAQ channel");
        return false;
    }

    m_rec_bufsize = rec_bufsize;
	m_rec_channel_log.reserve(m_rec_bufsize * m_nchan);
	m_rec_tstamp_log.reserve(m_nchan);

    if(chdaq_init())
    {
        m_b_initialised = true;
        return true;
    }
    else
    {
        ecl_error("ECL: ecl_daq::init - Could not initialize DAQ");
        return false;
    }
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_daq::start_sampling()
//	DESCRIPTION:	Start collecting DAQ samples
//--------------------------------------------------------------------
bool ecl_daq::start_sampling()
{
    if (m_b_sampling)
        {return true;}

    if (chdaq_start())
    {
        m_b_sampling = true;
        return true;
    }
    else
    {
        ecl_error("ECL: ecl_daq::start_sampling - Could not start DAQ sampling");
        return false;
    }
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_daq::get_samples()
//	DESCRIPTION:	Retrieve samples from the DAQ channels
//--------------------------------------------------------------------
long ecl_daq::get_samples(long * channels, long tstamp)
{
    if (!m_b_sampling)
    {
        ecl_error("ECL: ecl_daq::get_samples - Enable sampling before polling the DAQ");
        return false;
    }


    long res = chdaq_poll(channels, tstamp);
    if (res > 0)
    {
        if (m_b_recording)
        {
            if (m_rec_pos > m_rec_bufsize)
            {
                ecl_error("ECL: ecl_daq::get_samples - DAQ recording buffer is full");
                return false;
            }
            else
            {
				if (!tstamp) // if sampling unbuffered, keep count of missed polls
				{				
					if (!m_rec_pos) // first sample
					{
						tstamp = res;
					}
					else
					{
						tstamp = m_rec_tstamp_log.back() + res; //*(m_rec_tstamp_log+m_rec_pos-1) + res;
					}
				}
				
				m_rec_tstamp_log.push_back(tstamp);

				//*(m_rec_tstamp_log+m_rec_pos) = tstamp;
                
				for (int i = 0; i < m_nchan; i++)
				{					
					m_rec_channel_log.push_back(channels[i]);
					//*(m_rec_channel_log+(m_rec_pos*m_nchan)+i) = channels[i];
				}

				m_rec_pos++;
            }
        }
        return res;
    }
    else if (res < 0)
    {
        ecl_error("ECL: ecl_daq::get_samples - Error sampling DAQ");
        return res;
    }
    else
    {
        return res;
    }
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_daq::stop_sampling()
//	DESCRIPTION:	Stop collecting DAQ samples
//--------------------------------------------------------------------
bool ecl_daq::stop_sampling()
{
    if (!m_b_sampling)
        {return true;}

    if (chdaq_stop())
    {
        m_b_sampling = false;
        return true;
    }
    else
    {
        ecl_error("ECL: ecl_daq::stop_sampling - Could not stop DAQ sampling");
        return false;
    }
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_daq::start_recording()
//	DESCRIPTION:	Start recording samples
//--------------------------------------------------------------------
bool ecl_daq::start_recording(string fname)
{
    if (m_b_recording)
        {return true;}

    if (m_nchan < 1)
        {
            ecl_error("ECL: ecl_daq::start_recording - At least one channel is needed to record DAQ");
            return false;
        }

    if (m_rec_bufsize < 1)
        {
            ecl_error("ECL: ecl_daq::start_recording - Positive buffer size is needed to record DAQ");
            return false;
        }

    /*m_rec_channel_log = new (nothrow) long[m_rec_bufsize*m_nchan];
    m_rec_tstamp_log = new (nothrow) long[m_rec_bufsize*m_nchan];

    if (m_rec_channel_log == NULL || m_rec_tstamp_log == NULL)
        {
            ecl_error("ECL: ecl_daq::start_recording - Could not allocate memory for DAQ recording");
            return false;
        }*/

	m_rec_channel_log.clear();
	m_rec_tstamp_log.clear();

    m_rec_pos = 0;
    m_b_recording = true;
    m_rec_fname = fname;
    return true;
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_daq::stop_recording()
//	DESCRIPTION:	Stop recording samples
//--------------------------------------------------------------------
bool ecl_daq::stop_recording()
{
	if (m_b_recording)
	{
		FILE * m_rec_file = fopen(m_rec_fname.c_str(), "w+");
		if (m_rec_file)
		{
			for (int i=0; i<m_rec_pos;i++)
			{
				for (int j=0;j<m_nchan;j++)
				{
                    fprintf(m_rec_file,"%d ",m_rec_channel_log[(m_nchan*i)+j]);
				}
				fprintf(m_rec_file,"%d\n",m_rec_tstamp_log[i]);
			}
			fclose(m_rec_file);
		}
		else
		{
            ecl_error("ECL: ecl_daq::stop_recording - Could not write to DAQ recording file");
            return false;
		}

	}

	m_rec_channel_log.clear();
	m_rec_tstamp_log.clear();

	vector<long>().swap(m_rec_channel_log);
	vector<long>().swap(m_rec_tstamp_log);

	m_rec_file = NULL;
	m_rec_pos = 0;

	m_b_recording = false;
	return true;
}
//