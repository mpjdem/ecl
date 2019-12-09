#ifndef _ECL_OUTPUT_INCLUDED_
#define _ECL_OUTPUT_INCLUDED_

#include <vector>
#include <string>
#include <ctime>

#include "ecl_config.h"

using namespace std;

#define ECL_OUTPUT_COMMENT_BUFFER_SIZE						100


//--------------------------------------------------------------------
//  CLASS:			ecl_output
//	DESCRIPTION:	Output registration base class
//--------------------------------------------------------------------
class ecl_output
{
public:

															ecl_output();
	virtual												  ~ ecl_output();

	// Base class implemented
			bool											create_buffer(long bufsize);
	inline	bool											has_buffer(){return m_b_buffer_allocated;}
			bool											clear_buffer();
			bool											destroy_buffer();

	inline	void											set_fname(string fname){m_fname = fname;}

	inline void												set_daq_source(string daq_source){m_daq_source=daq_source;}
	inline void												set_experimenter_name(string exp_name){m_experimenter_name=exp_name;}
	inline void												set_subject_name(string subj_name){m_subject_name=subj_name;}
	inline void												set_session_id(string session_id){m_session_id=session_id;}
	inline void												set_monitor_name(string mon_name){m_monitor_name=mon_name;}

	inline void												set_monitor_h_px(long mon_hpx){m_monitor_h_px=mon_hpx;}
	inline void												set_monitor_w_px(long mon_wpx){m_monitor_w_px=mon_wpx;}
	inline void												set_monitor_h_mm(long mon_hmm){m_monitor_h_mm=mon_hmm;}
	inline void												set_monitor_w_mm(long mon_wmm){m_monitor_w_mm=mon_wmm;}
	inline void												set_monitor_distance(long mon_dist){m_monitor_distance=mon_dist;}
	inline void												set_monitor_framerate(long mon_fr){m_monitor_framerate=mon_fr;}
	inline void												set_monitor_bitdepth(long mon_bd){m_monitor_bitdepth=mon_bd;}

			bool											add_sample(long tstamp, long daq_x, long daq_y, long pix_x, long pix_y, vector<bool> status, vector<bool> stims, vector<bool> keys, vector<bool> flags);
			bool											add_comment(long tstamp, string comment_text);

    // Child class implemented
    virtual	bool											save_buffer(ecl_config * cfg)=0;


protected:

	// Base class implemented
			
			struct samplestruct
				{
						long		tstamp;

						short int	daq_x,
									daq_y, 
									pix_x, 
									pix_y;

						char		status;	

						short int	flags_stim,		
									flags_keys,
									flags_other;

						// Total: 19 bytes per sample
				};
	

			struct commentstruct
				{
						long	tstamp;
						string	comment_text;
				};


			string											m_fname;
			
			bool											m_b_buffer_allocated;
			unsigned long									m_bufsize;

			long											m_n_samples;
			vector<samplestruct>							m_samplebuf;

			vector<commentstruct>							m_commentbuf;

			string											m_daq_source;
			string											m_experimenter_name;
			string											m_subject_name;
			string											m_session_id;
			string											m_monitor_name;
			long											m_monitor_h_px;
			long											m_monitor_w_px;
			long											m_monitor_h_mm;
			long											m_monitor_w_mm;
			long											m_monitor_distance;
			long											m_monitor_framerate;
			long											m_monitor_bitdepth;

			time_t											m_first_sample_time;


    // Child class implemented
    // (none)
};

#endif	//_ECL_OUTPUT_INCLUDED_