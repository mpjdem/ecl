#ifndef _ECL_DAQ_NI_INCLUDED_
#define _ECL_DAQ_NI_INCLUDED_

#pragma warning(disable: 4786)

#include <vector>
#include <string>

#include <NIDAQmx.h>
#include "ecl_daq.h"

using namespace std;

#define NIDAQ_MINV      -5.12   // Volt
#define NIDAQ_MAXV      5.12    // Volt
#define NIDAQ_DEVN      1       // Device number
#define NIDAQ_SWBUF     5       // Software buffer, in seconds
#define NIDAQ_HWBUF     1       // Hardware buffer, in seconds
#define NIDAQ_MAXLAG    500     // Maximal number of samplings lagged

#pragma comment(lib, "NIDAQmx.lib")


//--------------------------------------------------------------------
//  CLASS:			ecl_daq_ni
//	DESCRIPTION:	ecl_daq child class for National Instruments cards
//--------------------------------------------------------------------
class  ecl_daq_ni : public ecl_daq
{
public:
															ecl_daq_ni();
	virtual												  ~ ecl_daq_ni();

    // Base class implementations
    virtual bool                                            chdaq_init();
    virtual long											chdaq_poll(long * channels, long tstamp);
    virtual bool                                            chdaq_start();
    virtual bool                                            chdaq_stop();

    // Own implementations
    // (none)


protected:
    // Base class implementations
    // (none)

    // Own implementations
			TaskHandle										m_taskhandle;
			float64										*	m_hwbuf;
			char											m_errbuf[4096];
			long											m_read;
			unsigned long									m_avail;
};
//

#endif	//_ECL_DAQ_NI_INCLUDED_
