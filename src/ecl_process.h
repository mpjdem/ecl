#ifndef _ECL_PROCESS_INCLUDED_
#define _ECL_PROCESS_INCLUDED_

#include <vector>
#include <string>

#include "ecl_config.h"
#include "ecl_datastruct.h"
#include "ecl_daq.h"
#include "ecl_monitor.h"
#include "ecl_output.h"
#include "ecl_output.h"
#include "ecl_region.h"
#include "ecl_response.h"

using namespace std;

#define ECL_CAL_POINT_SIZE 3
#define ECL_MSGBUF_MAXL 100
#define ECL_CTRL_SCALE_H 0.75
#define ECL_CTRL_BORDER 2

//--------------------------------------------------------------------
//  CLASS:			ecl_process
//	DESCRIPTION:	ecl_process default class
//--------------------------------------------------------------------
class ecl_process
{

public:

															ecl_process();
	virtual												  ~ ecl_process();


	// Initialisation
			bool											init(ecl_config dpi_cfg, ecl_daq *dpi_daq, ecl_response *subj_resp=NULL, ecl_response *contr_resp=NULL, ecl_monitor *subj_mon=NULL, ecl_monitor *ctrl_mon=NULL);
	inline	bool											is_initialised(){return m_b_initialised;}

			bool											has_datastruct();
			bool											attach_datastruct(ecl_datastruct * datastruct);
			bool											detach_datastruct();


	// Calibration
			bool											calibrate(ecl_response *subj_resp=NULL, ecl_response *ctrl_resp=NULL);
			bool											drift_correction(long pix_x, long pix_y);
			inline	bool									is_calibrated(){return m_b_calibrated;}


	// Sampling
			bool											start_sampling(string mode="");
			bool											stop_sampling();
			bool											sample(long tstamp=0);
	inline	long											get_sampling_time(){return m_daq_time;}
			bool											keep_sampling(long t, string mode);

	// Eye status
	inline	long											get_eye_x(){return m_status_pix_x;}
	inline	long											get_eye_y(){return m_status_pix_y;}

	inline	bool											is_in_oscillation(){return !m_b_status_saccade && !m_b_status_safe;}
	inline	bool											is_in_fixation(){return !m_b_status_saccade && m_b_status_safe;}
	inline	bool											is_in_saccade(){return m_b_status_saccade;}
	inline	bool											is_in_falselock(){return !m_b_status_good;}

	inline	bool											is_in_safe_fixation(){return !m_b_status_saccade && m_b_status_safe && m_b_status_good;}
	inline	bool											is_in_safe_saccade(){return m_b_status_saccade && m_b_status_safe && m_b_status_good;}

	inline	bool											is_in_blink(){return m_b_status_blink;}
	inline	bool											is_tracking(){return m_b_status_track;}
	inline	bool											is_in_sync(){return !m_b_status_timeout;}


	// Other status variables
            void                                            add_stim_flag(long n=1);
			void                                            add_key_flag(long n=1);
            void                                            add_key_flag(ecl_response * resp_dev, string resp_button);
            void                                            add_other_flag(long n=1);

            void											stim_flag_on(long n){m_b_status_stim[n-1]=true;}
            void											key_down(long n){m_b_status_key[n-1]=true;}
            void											flag_on(long n){m_b_status_flag[n-1]=true;}

            void											stim_flag_off(long n){m_b_status_stim[n-1]=false;}
            void											key_up(long n){m_b_status_key[n-1]=false;}
            void											flag_off(long n){m_b_status_flag[n-1]=false;}


	// Output
			bool											attach_output_file(ecl_output & output_file);
			bool											detach_output_file();
			bool											save_output_file();
			bool											clear_output_buffer();
	inline	bool											has_output_file(){return m_b_output_file_registered;}


	// Registered monitors and response buttons are publicly accessible
			ecl_monitor									*	m_subj_mon;
			ecl_monitor									*	m_ctrl_mon;
			ecl_response								*	m_subj_resp;
			ecl_response								*	m_ctrl_resp;


	// Attach a datastruct
			ecl_datastruct								*	m_datastruct;


	// Debug and status messages
			bool											add_to_msgbuf(string msg);
			bool											add_to_msgbuf(string msg, long max_n);
			string											get_from_msgbuf(unsigned long n);
			bool											clear_msgbuf();
			bool											prune_msgbuf(unsigned long n);
			bool											draw_msgbuf(long x0, long y0, long h, long w, ecl_LUTc fg_c=-1, ecl_LUTc bg_c=-1, ecl_monitor * mon = NULL);
			

	// Regions and drawing
			unsigned long									add_region(ecl_region *reg_p, string reg_name);
			ecl_region									*	get_region_pointer(unsigned long handle);
			ecl_region									*	get_region_pointer(string reg_name);
			unsigned long									get_region_handle(string reg_name);
			string											get_region_name(unsigned long handle);
			bool											on_region(unsigned long handle);
			bool											on_region(string reg_name);
			unsigned long									find_region();
			bool											clear_all_regions();

	// Control screen functions
			bool											ctrl_update_screen();

			bool											ctrl_enable_region(unsigned long handle);
			bool											ctrl_disable_region(unsigned long handle);
			bool											ctrl_enable_region(string reg_name);
			bool											ctrl_disable_region(string reg_name);	
			
			
	// Monitor the status of all devices connected to ecl_process
	//		bool											monitor();


protected:

	// Configuration variables
			bool											m_b_initialised;
			bool											m_b_calibrated;
			ecl_config										m_config;


	// DAQ device
			ecl_daq										*	m_daq;
			long											m_daq_time;


	// Calibration sub-functions and temporary variables
			bool											construct_calibration_grid(string dotpattern="");
			bool											add_calibration_point(long pix_x, long pix_y, long daq_x, long daq_y);
			bool											replace_calibration_point(long n, long daq_x, long daq_y);
			bool											measure_calibration_point(long n, ecl_response * subj_resp, ecl_response * ctrl_resp);
			bool											fit_calibration();
			bool											save_calibration_to_config();

			vector<long>									m_cal_tmp_pix_x;
			vector<long>									m_cal_tmp_pix_y;
			vector<long>									m_cal_tmp_daq_x;
			vector<long>									m_cal_tmp_daq_y;
			vector<bool>									m_cal_tmp_good;
			vector<double>									m_cal_tmp_params;
			vector<double>									m_cal_tmp_fit;

			RECT											m_cal_rect;
			RECT                                            m_flocktol_rect;
			
			// These are only used by the calibrate function()
			vector<long>									m_cal_grid_x;
			vector<long>									m_cal_grid_y;


	// Conversion functions
			long											daq2pix_x(long daq_x);
			long											daq2pix_y(long daq_y);
			long											pix2daq_x(long pix_x);
			long											pix2daq_y(long pix_y);


	// Non-blocking sampling function
			//DWORD WINAPI									keep_sampling_async(LPVOID args);


	// Status variables
			long										    m_daq_channels[4];

			long											m_status_daq_x;
			long											m_status_daq_y;
			long											m_status_pix_x;
			long											m_status_pix_y;

			bool											m_b_status_saccade;
			bool											m_b_status_safe;
			bool											m_b_status_good;
			bool											m_b_status_blink;
			bool											m_b_status_track;
			bool											m_b_status_timeout;

			vector<bool>									m_b_status_stim;
			vector<string>									m_s_tracked_keys;
			vector<ecl_response*>                           m_p_tracked_keys;
			vector<long>									m_h_tracked_keys;
			vector<bool>									m_b_status_key;
			vector<bool>									m_b_status_flag;

			bool											m_async_sampling_status;


	// Output file
			bool											m_b_output_file_registered;
			ecl_output									*	m_output_file;


	// Message buffer
			vector<string>									m_msg_buf;


	// Regions
			vector<ecl_region *>							m_regions_pointer;
			vector<string>									m_regions_name;
	
	// Control screen
			bool											ctrl_initialise();
			bool											ctrl_draw_eye(long x, long y);
			bool											ctrl_draw_status(long x, long y);

			vector<bool>									m_ctrl_regions_status;
			long											m_ctrl_screen_preload_h;
			long											m_ctrl_eye_preload_h;

			long											m_ctrl_update_freq;
			long											m_ctrl_last_update_t;

			RECT											m_ctrl_screen_main_area;
			RECT											m_ctrl_screen_msg_area;
			RECT											m_ctrl_screen_info_area;

			vector<string>									m_ctrl_info;
};
//

#endif // _ECL_PROCESS_INCLUDED_
