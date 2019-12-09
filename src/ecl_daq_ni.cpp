#pragma warning(disable: 4786)

#include "stdafx.h"
#include <new>

#include "ecl.h"
#include "ecl_daq_ni.h"

using namespace std;


//--------------------------------------------------------------------
//	FUNCTION:		ecl_daq_ni::ecl_daq_ni()
//	DESCRIPTION:	Constructor
//--------------------------------------------------------------------
ecl_daq_ni::ecl_daq_ni()
{
    m_taskhandle = 0;
	m_hwbuf = NULL;
	m_errbuf[0] = 0;
	m_read = 0;
	m_avail = 0;
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_daq_ni::~ecl_daq_ni()
//	DESCRIPTION:	Deconstructor
//--------------------------------------------------------------------
ecl_daq_ni::~ecl_daq_ni()
{
	chdaq_stop();
	DAQmxClearTask(m_taskhandle);
	m_taskhandle = 0;
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_daq_ni::chdaq_init()
//	DESCRIPTION:	NI card initialization routine
//--------------------------------------------------------------------
bool ecl_daq_ni::chdaq_init()
{
	// Create a task
	if (DAQmxFailed(DAQmxCreateTask("",&m_taskhandle)))
		{return false;}

	// Add the channels
	char chan_name[20];

	for (int i=0; i<m_nchan; i++)
	{
	    if(sprintf(chan_name,"Dev%d/ai%d",NIDAQ_DEVN,i) != 8)
	    {
	        DAQmxClearTask(m_taskhandle);
            m_taskhandle = 0;
            return false;
	    }

	    if (DAQmxFailed(DAQmxCreateAIVoltageChan(m_taskhandle,chan_name,"",DAQmx_Val_RSE,NIDAQ_MINV,NIDAQ_MAXV,DAQmx_Val_Volts,NULL)))
	    {
            DAQmxClearTask(m_taskhandle);
            m_taskhandle = 0;
            return false;
	    }
	}

    // Configure the sampling timing
    int ni_buf_size = ECL_SAMPLE_FREQ * m_nchan * NIDAQ_HWBUF;
	if (DAQmxFailed(DAQmxCfgSampClkTiming(m_taskhandle,"",ECL_SAMPLE_FREQ,DAQmx_Val_Rising,DAQmx_Val_ContSamps,ni_buf_size)) )
	{
		DAQmxClearTask(m_taskhandle);
		m_taskhandle = 0;
		return false;
	}

    // Success!
	return true;
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_daq_ni::chdaq_start()
//	DESCRIPTION:	Start sampling NI card
//--------------------------------------------------------------------
bool ecl_daq_ni::chdaq_start()
{
	if (m_taskhandle == 0)
		{return false;}

	if (m_hwbuf == NULL)
	{
		m_hwbuf = new (nothrow) float64[ECL_SAMPLE_FREQ * m_nchan * NIDAQ_HWBUF];
	}
	else
	{
		ecl_die("ECL: ecl_daq_ni::chdaq_start - Delete buffer before cretaing a new one");
		return false;
	}

	return !DAQmxFailed(DAQmxStartTask(m_taskhandle));
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_daq_ni::chdaq_stop()
//	DESCRIPTION:	Stop sampling NI card
//--------------------------------------------------------------------
bool ecl_daq_ni::chdaq_stop()
{
	if (m_taskhandle == 0)
        {return false;}

	if (m_hwbuf != NULL)
	{
		delete [] m_hwbuf;
		m_hwbuf = NULL;
	}

	return !DAQmxFailed(DAQmxStopTask(m_taskhandle));
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_daq_ni::chdaq_poll()
//	DESCRIPTION:	Retrieve samples from NI card
//--------------------------------------------------------------------
long ecl_daq_ni::chdaq_poll(long * channels, long tstamp)
{
	if (m_taskhandle == 0)
		{return -1;}

	int32 res;
	bool failed;

	DAQmxGetReadAvailSampPerChan(m_taskhandle, &m_avail);

	if (m_avail < 1)
	{
	    return 0;
	}
	else if (m_avail < NIDAQ_MAXLAG*m_nchan)
	{
        // If unbuffered, take the latest sample and return the lag
	    if(tstamp == 0)
	    {
            res = DAQmxReadAnalogF64(m_taskhandle,m_avail,0.0,DAQmx_Val_GroupByScanNumber,m_hwbuf,ECL_SAMPLE_FREQ * m_nchan * NIDAQ_HWBUF,&m_read,NULL);
            failed = DAQmxFailed(res);

            if (failed)
            {
                DAQmxGetErrorString(res, m_errbuf, 4095);
                chdaq_stop();
                m_taskhandle = 0;
                m_read = 0;
                m_avail = 0;
                ecl_die(m_errbuf);
                return -1;
            }
			else
			{				
				for (int i=0; i<m_nchan; i++)
				{
					channels[i] = (long)(DAQ_MINVAL + ((((m_hwbuf[i]-NIDAQ_MINV)/(NIDAQ_MAXV-NIDAQ_MINV)))*(DAQ_MAXVAL-DAQ_MINVAL)));
				}
			}

            return m_avail;
	    }
	    // If buffered, add these samples to the software buffer (thus: need to declare a software buffer)
	    else if (tstamp > 0)
	    {
            return m_avail;
			// If the software buffer is full (too much accumulated lag), return an error

            // If not, take the sample corresponding to the tstamp out of it (thus: need to remember how many samples were already removed from the buffer)

            // If tstamp exceeds the number of available samples, return 0 (not available yet)
            // If tstamp equals the first and only number in the software buffer, return 1 ('everything is on time')
            // If tstamp is not the latest sample, return the lag accumulated since the last sampling call (thus: need to keep the previous size of the software buffer somewhere)
	    }
	    else
	    {
            chdaq_stop();
            m_taskhandle = 0;
            m_read = 0;
            m_avail = 0;
            ecl_die("ECL: ecl_daq_ni::chdaq_poll() - Invalid timestamp)");
            return -1;
	    }
	}
	else
	{
        chdaq_stop();
        ecl_die("ECL: ecl_daq_ni::chdaq_poll - Time-out since the last DAQ sampling call");
        return -1;
	}
}
//
