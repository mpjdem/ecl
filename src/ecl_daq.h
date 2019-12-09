#ifndef _ECL_DAQ_INCLUDED_
#define _ECL_DAQ_INCLUDED_

#include <vector>
#include <string>

using namespace std;

#define DAQ_MINVAL  0
#define DAQ_MAXVAL  4096

//--------------------------------------------------------------------
//  CLASS:			ecl_daq
//	DESCRIPTION:	Data AcQuisition base class
//--------------------------------------------------------------------
class ecl_daq
{
public:

															ecl_daq();
	virtual												  ~ ecl_daq();

	// Base class implemented
            bool											init(long nchan, long rec_bufsize=0);
    inline	bool                                            is_initialised(){return m_b_initialised;}
    inline  long                                            get_nchan(){return m_nchan;}

            bool                                            start_sampling();
            long                                            get_samples(long * channels, long tstamp=0);
            bool											is_sampling(){return m_b_sampling;}
            bool                                            stop_sampling();

			bool											start_recording(string fname);
			bool											stop_recording();

    // Child class implemented
    // (none)


protected:

    // Base class implemented
            long										    m_nchan;
            long                                            m_freq;
            bool                                            m_b_initialised;
            bool                                            m_b_sampling;

			bool											m_b_recording;
            long                                            m_rec_bufsize;
			FILE										  * m_rec_file;
			string											m_rec_fname;
			vector<long>									m_rec_channel_log;
			vector<long>									m_rec_tstamp_log;
			long                                            m_rec_pos;

    // Child class implemented
    virtual bool                                            chdaq_init()=0;
    virtual long											chdaq_poll(long * channels, long tstamp)=0;
    virtual bool                                            chdaq_start()=0;
    virtual bool                                            chdaq_stop()=0;
};
//

#endif	//_ECL_DAQ_INCLUDED_
