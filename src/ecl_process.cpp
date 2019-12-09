#include "stdafx.h"

#include <cstdio>
#include <math.h>

#include "ecl.h"
#include "ecl_process.h"

using namespace std;


//--------------------------------------------------------------------
//	FUNCTION:		ecl_process::ecl_process()
//	DESCRIPTION:	Constructor
//--------------------------------------------------------------------
ecl_process::ecl_process()
{
    m_b_initialised = false;
    m_b_calibrated = false;
    m_daq = NULL;
    m_subj_resp = NULL;
    m_ctrl_resp = NULL;
    m_subj_mon = NULL;
    m_ctrl_mon = NULL;

    m_daq_time = 0;
    //m_daq_channels = NULL;

    m_status_daq_x = 0;
	m_status_daq_y = 0;
    m_status_pix_x = 0;
	m_status_pix_y = 0;

	m_b_status_saccade = false;
	m_b_status_safe = false;
	m_b_status_good = false;
	m_b_status_blink = false;
	m_b_status_track = false;
	m_b_status_timeout = true;

    m_b_status_stim.clear();
	m_b_status_key.clear();
	m_b_status_flag.clear();

	m_cal_tmp_pix_x.clear();
	m_cal_tmp_pix_y.clear();
	m_cal_tmp_daq_x.clear();
	m_cal_tmp_daq_y.clear();
	m_cal_tmp_good.clear();
	m_cal_tmp_params.clear();
	m_cal_tmp_fit.clear();

	m_cal_grid_x.clear();
	m_cal_grid_y.clear();

	m_async_sampling_status = false;

	m_b_output_file_registered = false;
	m_output_file = NULL;

	m_ctrl_screen_preload_h = 0;
	m_ctrl_eye_preload_h = 0;
	m_ctrl_update_freq = 20;
	m_ctrl_last_update_t = 0;
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_process::~ecl_process()
//	DESCRIPTION:	Deconstructor
//--------------------------------------------------------------------
ecl_process::~ecl_process()
{

}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_process::init()
//	DESCRIPTION:	Initialisation routine
//--------------------------------------------------------------------
bool ecl_process::init(ecl_config dpi_cfg, ecl_daq *dpi_daq, ecl_response *subj_resp, ecl_response *contr_resp, ecl_monitor *subj_mon, ecl_monitor *contr_mon)
{
    if(m_b_initialised)
        {return true;}

    m_config = dpi_cfg;
    m_daq = dpi_daq;
    m_subj_resp = subj_resp;
    m_ctrl_resp = contr_resp;
    m_subj_mon = subj_mon;
    m_ctrl_mon = contr_mon;

    if( !m_config.is_initialised() || !m_daq->is_initialised() ) 
	{
        ecl_error("ECL: ecl_process::init() - Initialize all devices first");
        return false;
    }
	
	if (m_subj_resp != NULL && !m_subj_resp->is_initialised())
	{
        ecl_error("ECL: ecl_process::init() - Initialize all devices first");
        return false;
	}

	if (m_ctrl_resp != NULL && !m_ctrl_resp->is_initialised())
	{
        ecl_error("ECL: ecl_process::init() - Initialize all devices first");
        return false;
	}

	if (m_subj_mon != NULL && !m_subj_mon->is_initialised())
	{
        ecl_error("ECL: ecl_process::init() - Initialize all devices first");
        return false;
	}

	if (m_ctrl_mon != NULL && !m_ctrl_mon->is_initialised())
	{
        ecl_error("ECL: ecl_process::init() - Initialize all devices first");
        return false;
	}


    m_daq_time = 0;
    //m_daq_channels = NULL;

    m_status_daq_x = 0;
	m_status_daq_y = 0;
    m_status_pix_x = 0;
	m_status_pix_y = 0;

	m_b_status_saccade = false;
	m_b_status_safe = false;
	m_b_status_good = false;
	m_b_status_blink = false;
	m_b_status_track = false;
	m_b_status_timeout = true;

	m_b_status_stim.reserve(1);
	m_b_status_key.reserve(2);
	if (m_subj_resp != NULL)
	{
	    m_b_status_key.push_back(false); // automatically allocate the main subject button here, if it exists
	}
	m_b_status_flag.reserve(2);

    long n = m_config.m_cal_points_n;
	m_cal_tmp_pix_x.reserve(n);
	m_cal_tmp_pix_y.reserve(n);
	m_cal_tmp_daq_x.reserve(n);
	m_cal_tmp_daq_y.reserve(n);
	m_cal_tmp_good.reserve(n);
	m_cal_tmp_params.reserve(m_config.m_cal_params.size());
	m_cal_tmp_fit.reserve(m_config.m_cal_fit.size());
	
	for (int i=0; i<n; i++)	
	{
		m_cal_tmp_pix_x[i] = 0;
		m_cal_tmp_pix_y[i] = 0;
		m_cal_tmp_daq_x[i] = 0;
		m_cal_tmp_daq_y[i] = 0;
		m_cal_tmp_good[i] = false;
	}

	for (int i=0; i<m_config.m_cal_params.size(); i++)	
		m_cal_tmp_params[i] = 0.0;

	for (int i=0; i<m_config.m_cal_fit.size(); i++)	
		m_cal_tmp_fit[i] = 0.0;

    if (m_subj_mon != NULL)
    {
		double flocktol_degrange_x = m_config.m_cal_range_deg_x + (m_config.m_cal_flocktol_deg_x*2);
        double flocktol_degrange_y = m_config.m_cal_range_deg_y + (m_config.m_cal_flocktol_deg_y*2);
        long flocktol_pixrange_x = (long)floor(0.5+(tan((flocktol_degrange_x*PI)/180)*m_subj_mon->get_distance_mm()));
        long flocktol_pixrange_y = (long)floor(0.5+(tan((flocktol_degrange_y*PI)/180)*m_subj_mon->get_distance_mm()));
		long center_offset_pix_x = (long)floor(0.5+(tan((m_config.m_cal_center_offset_deg_x*PI)/180)*m_subj_mon->get_distance_mm()));
		long center_offset_pix_y = (long)floor(0.5+(tan((m_config.m_cal_center_offset_deg_y*PI)/180)*m_subj_mon->get_distance_mm()));

        m_flocktol_rect.left = (m_subj_mon->get_width_pix()/2) - (flocktol_pixrange_x/2) + center_offset_pix_x;
        m_flocktol_rect.right = (m_subj_mon->get_width_pix()/2) + (flocktol_pixrange_x/2) + center_offset_pix_y;
        m_flocktol_rect.top = (m_subj_mon->get_height_pix()/2) - (flocktol_pixrange_y/2) + center_offset_pix_x;
        m_flocktol_rect.bottom = (m_subj_mon->get_height_pix()/2) + (flocktol_pixrange_y/2) + center_offset_pix_y;

        long cal_pixrange_x = (long)floor(0.5+(tan((m_config.m_cal_range_deg_x*PI)/180)*m_subj_mon->get_distance_mm()));
        long cal_pixrange_y = (long)floor(0.5+(tan((m_config.m_cal_range_deg_y*PI)/180)*m_subj_mon->get_distance_mm()));

        m_cal_rect.left = (m_subj_mon->get_width_pix()/2) - (cal_pixrange_x/2) + center_offset_pix_x;
        m_cal_rect.right = (m_subj_mon->get_width_pix()/2) + (cal_pixrange_x/2) + center_offset_pix_y;
        m_cal_rect.top = (m_subj_mon->get_height_pix()/2) - (cal_pixrange_y/2) + center_offset_pix_x;
        m_cal_rect.bottom = (m_subj_mon->get_height_pix()/2) + (cal_pixrange_y/2) +center_offset_pix_y;
    }
    else
    {
        m_flocktol_rect.left = 0;
        m_flocktol_rect.right = 0;
        m_flocktol_rect.top = 0;
        m_flocktol_rect.bottom = 0;

        m_cal_rect.left = 0;
        m_cal_rect.right = 0;
        m_cal_rect.top = 0;
        m_cal_rect.bottom = 0;
    }

    // If the config contains non-zero calibration values, use these to perform an automatic calibration

    m_b_calibrated = true;
	m_b_initialised = true;
	m_async_sampling_status = false;

	if (m_ctrl_mon != NULL && !ctrl_initialise())
	{
		ecl_error("ECL: ecl_process::init() - Could not initialize control screen");
		return false;
	}

	return true;
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_process::start_sampling()
//	DESCRIPTION:	Start sampling the DAQ
//--------------------------------------------------------------------
bool ecl_process::start_sampling(string mode)
{
    if(!m_b_initialised)
    {
        ecl_error("ECL: ecl_process::start_sampling()) - Initialize ecl_process first");
        return false;
    }
	
	if (m_daq->is_sampling())
    {return true;}

    m_daq_time = 0;

	m_daq_channels[0] = 0;
	m_daq_channels[1] = 0;
	m_daq_channels[2] = 0;
	m_daq_channels[3] = 0;

    m_status_daq_x = 0;
	m_status_daq_y = 0;
    m_status_pix_x = 0;
	m_status_pix_y = 0;

	m_b_status_saccade = false;
	m_b_status_safe = false;
	m_b_status_good = false;
	m_b_status_blink = false;
	m_b_status_track = false;
	m_b_status_timeout = true;

    vector<bool>::iterator it;

    for (it=m_b_status_stim.begin(); it < m_b_status_stim.end(); it++)
	{
		*it = false;
	}

    for (it=m_b_status_key.begin(); it < m_b_status_key.end(); it++)
	{
		*it = false;
	}

    for (it=m_b_status_flag.begin(); it < m_b_status_flag.end(); it++)
	{
		*it = false;
	}


    bool res = m_daq->start_sampling();

    if(!res)
    {return false;}

   /* if (mode == "AUTO")
    {
        m_async_sampling_status = keep_sampling_async();
    }*/

    return true;
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_process::stop_sampling()
//	DESCRIPTION:	Stop sampling the DAQ
//--------------------------------------------------------------------
bool ecl_process::stop_sampling()
{
    if (!m_daq->is_sampling())
    {return true;}

    /*if (mode == "AUTO") // if async sampling, stop that thread
    {
        m_async_sampling_status = keep_sampling_async();
    }*/

    bool res = m_daq->stop_sampling();

    if(!res)
    {return false;}

    m_daq_time = 0;

	m_daq_channels[0] = 0;
	m_daq_channels[1] = 0;
	m_daq_channels[2] = 0;
	m_daq_channels[3] = 0;

    m_status_daq_x = 0;
	m_status_daq_y = 0;
    m_status_pix_x = 0;
	m_status_pix_y = 0;

	m_b_status_saccade = false;
	m_b_status_safe = false;
	m_b_status_good = false;
	m_b_status_blink = false;
	m_b_status_track = false;
	m_b_status_timeout = true;

    vector<bool>::iterator it;

    for (it=m_b_status_stim.begin(); it < m_b_status_stim.end(); it++)
	{
		*it = false;
	}

    for (it=m_b_status_key.begin(); it < m_b_status_key.end(); it++)
	{
		*it = false;
	}

    for (it=m_b_status_flag.begin(); it < m_b_status_flag.end(); it++)
	{
		*it = false;
	}

    /*if(m_b_output_file_registered)
	{
		m_output_file->save_buffer(&m_config);
		m_output_file->clear_buffer();
	}*/

    return true;
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_process::calibrate()
//	DESCRIPTION:	Main calibration routine
//--------------------------------------------------------------------
bool ecl_process::calibrate(ecl_response *subj_resp, ecl_response *ctrl_resp)
{
    // Error checking
	if(!m_b_initialised)
    {
        ecl_error("ECL: ecl_process::calibrate()) - Initialize ecl_process first");
        return false;
    }

	if ((subj_resp == NULL && m_subj_resp == NULL) || (ctrl_resp == NULL && m_ctrl_resp == NULL))
	{
		ecl_error("ECL: ecl_process::calibrate()) - Missing response device");
        return false;
	}

	if ((subj_resp != NULL && !subj_resp->is_initialised()) || (ctrl_resp != NULL && !ctrl_resp->is_initialised()))
	{
		ecl_error("ECL: ecl_process::calibrate()) - Response device not initialised");
        return false;
	}

	if (m_subj_mon == NULL)
	{
		ecl_error("ECL: ecl_process::calibrate()) - Missing monitor device");
        return false;
	}

	// Retrieve non-specified devices
	if (!subj_resp)
		{subj_resp = m_subj_resp;}
	if (!ctrl_resp)
		{ctrl_resp = m_ctrl_resp;}

	// Remember status from before calibration
	ecl_output * had_file = NULL;
	bool was_sampling = false;
	vector<bool> reg_stat;

	if(m_daq->is_sampling())
	{
		stop_sampling();
		was_sampling = true;
	}

	if(has_output_file())
	{
		had_file = m_output_file;
		clear_output_buffer();
		detach_output_file();
	}

	for (int i=0; i<m_ctrl_regions_status.size(); i++)
	{
		reg_stat.push_back(m_ctrl_regions_status[i]);
		m_ctrl_regions_status[i] = false;
	}


	// Retrieve some useful variables
	long ctrl_x0, ctrl_y0, ctrl_w, ctrl_h, ctrl_xmid, ctrl_ymid;
	
	if (m_ctrl_mon && m_ctrl_screen_preload_h)
	{
		ctrl_x0 = m_ctrl_screen_main_area.left;
		ctrl_y0 = m_ctrl_screen_main_area.top;
		ctrl_w = m_ctrl_screen_main_area.right - m_ctrl_screen_main_area.left;
		ctrl_h = m_ctrl_screen_main_area.bottom - m_ctrl_screen_main_area.top;
		ctrl_xmid = ctrl_x0 + (ctrl_w/2);
		ctrl_ymid = ctrl_y0 + (ctrl_h/2);
	}

	long ctrl_pxx, ctrl_pxy;
	
	long subj_w = m_subj_mon->get_width_pix();
	long subj_h = m_subj_mon->get_height_pix();

	string sjr0, sjr1, ctr0, ctr1, ctr2;
	char * msg;

	sjr0 = subj_resp->get_name(0);
	sjr1 = subj_resp->get_name(1);
	ctr0 = ctrl_resp->get_name(0);
	ctr1 = ctrl_resp->get_name(1);
	ctr2 = ctrl_resp->get_name(2);

	// Display screen for setting up the machine
	while (!ctrl_resp->is_pressed(0))
	{
		m_subj_mon->clear_screen();
		m_subj_mon->draw_oval(0,subj_w/2,subj_h/2,ECL_CAL_POINT_SIZE,ECL_CAL_POINT_SIZE);
		m_subj_mon->present();
	}

	if (m_ctrl_mon)
	{
		sprintf(msg,"Press %s to continue.",ctr0);
		add_to_msgbuf(msg);
		ctrl_update_screen();
		m_ctrl_mon->present();
	}

	// Go to calibration
	if(!construct_calibration_grid())
	{
		ecl_error("ECL: ecl_process::calibrate()) - Could not construct calibration grid");
        return false;
	}

	if(!m_daq->start_sampling())
	{
		ecl_error("ECL: ecl_process::calibrate()) - Could not sample DAQ device");
		return false;
	}

	long cal_pxx, cal_pxy;
	bool good;
	bool done = false;
	bool repeat_calibration = false;
	vector<unsigned long> p_to_measure;
	long bad_n,cal_p;
	
	while (repeat_calibration)
	{
		// Determine which points to measure
		bad_n = 0;
		p_to_measure.clear();
		for (int i=0; i<m_cal_tmp_good.size();i++)
		{
			if(!m_cal_tmp_good[i])
			{
				p_to_measure[bad_n]=i;
				bad_n++;
			}
		}

		// Measure them
		for (int i = 0; i < p_to_measure.size(); i++)
		{
			// Retrieve pixel value, do some checks
			cal_p = p_to_measure[i];
			cal_pxx = m_cal_grid_x[cal_p];
			cal_pxy = m_cal_grid_y[cal_p];

			if(cal_pxx < m_cal_rect.left || cal_pxx > m_cal_rect.right || cal_pxy < m_cal_rect.top || cal_pxy > m_cal_rect.bottom)
			{
				ecl_error("ECL: ecl_process::calibrate()) - Calibration point does not match calibration rectangle");
				return false;
			}

			// Draw this point onto the subject monitor, and start sampling
			m_subj_mon->clear_screen();
			m_subj_mon->draw_oval(0,cal_pxx,cal_pxy,ECL_CAL_POINT_SIZE,ECL_CAL_POINT_SIZE);
			m_subj_mon->present();

			good = measure_calibration_point(cal_p,subj_resp,ctrl_resp);

			// If the point is not good, offer a choice
			if(!good)
			{
				m_daq->stop_sampling();

				if (m_ctrl_mon)
				{
					sprintf(msg,"BAD POINT. Press %s to repeat, %s to skip.",ctr0,ctr1);
					add_to_msgbuf(msg);
					ctrl_update_screen();
					m_ctrl_mon->present();
				}

				while (!done)
				{
					if (m_ctrl_resp->is_pressed(0))
						{done = true;}

					if (m_ctrl_resp->is_pressed(1))
						{cal_p--; done = true;}
				}

				done = false;

				if(!m_daq->start_sampling())
				{
					ecl_error("ECL: ecl_process::calibrate()) - Could not sample DAQ device");
					return false;
				}
			}	

			// Continue
		}

		// All points sampled; now fit them
		m_daq->stop_sampling();

		if(!fit_calibration() && m_ctrl_mon)
		{
			add_to_msgbuf("Fit failed. No changes were made to the calibration.");
			ctrl_update_screen();
			m_ctrl_mon->present();
		}

		// Show a graphical display of the fit, indicating good and bad points
		if (m_ctrl_mon && m_ctrl_screen_preload_h)
		{	
			// Prepare surface
			m_ctrl_mon->draw_rect(m_ctrl_screen_preload_h,ctrl_xmid,ctrl_ymid,ctrl_w,ctrl_h,m_ctrl_mon->get_bg_color());

			// Presented points
			for (int i=0; i<m_cal_grid_x.size(); i++)
			{
				ctrl_pxx = ctrl_x0 + (long)(((double)m_cal_grid_x[i]/subj_w)*(double)ctrl_w);
				ctrl_pxy = ctrl_y0 + (long)(((double)m_cal_grid_y[i]/subj_h)*(double)ctrl_h);
				m_ctrl_mon->draw_oval(m_ctrl_screen_preload_h,ctrl_pxx,ctrl_pxy,ECL_CAL_POINT_SIZE,ECL_CAL_POINT_SIZE,RGB(200,200,200));
			}
		
			// Fitted measurements, good points in blue, bad points in red
			if(m_b_calibrated || fit_calibration())
			{
				for (int i=0; i<m_cal_grid_x.size(); i++)
				{
					ctrl_pxx = ctrl_x0 + (long)(((double)m_cal_tmp_pix_x[i]/subj_w)*(double)ctrl_w);
					ctrl_pxy = ctrl_y0 + (long)(((double)m_cal_tmp_pix_y[i]/subj_h)*(double)ctrl_h);	
					
					if (m_cal_tmp_good[i])
						m_ctrl_mon->draw_oval(m_ctrl_screen_preload_h,ctrl_pxx,ctrl_pxy,ECL_CAL_POINT_SIZE,ECL_CAL_POINT_SIZE,RGB(0,0,200));
					else
						m_ctrl_mon->draw_oval(m_ctrl_screen_preload_h,ctrl_pxx,ctrl_pxy,ECL_CAL_POINT_SIZE,ECL_CAL_POINT_SIZE,RGB(200,00,0));
				}

				// Present to the screen
				m_ctrl_mon->blit_preloaded_image(m_ctrl_screen_preload_h,m_ctrl_mon->get_width_pix()/2,m_ctrl_mon->get_height_pix()/2);
				m_ctrl_mon->present();
			}

			// Offer a choice to resample all points, resample bad points, or continue
			if (m_ctrl_mon)
			{
				sprintf(msg,"Resample all points (%s), bad points (%s), or continue (%s)?",ctr0,ctr1,ctr2);
				add_to_msgbuf(msg);
				ctrl_update_screen();
				m_ctrl_mon->present();
			}
		}

		while (!done)
		{
			if (m_ctrl_resp->is_pressed(0))
			{
				for (int i=0; i<m_cal_tmp_pix_x.size(); i++)
				{
					m_cal_tmp_pix_x[i] = 0;
					m_cal_tmp_pix_y[i] = 0;
					m_cal_tmp_daq_x[i] = 0;
					m_cal_tmp_daq_y[i] = 0;
					m_cal_tmp_good[i] = false;
				}

				for (int i=0; i<m_config.m_cal_params.size(); i++)	
					m_cal_tmp_params[i] = 0.0;

				for (int i=0; i<m_config.m_cal_fit.size(); i++)	
					m_cal_tmp_fit[i] = 0.0;
				
				repeat_calibration = true;
				done = true;
			}

			if (m_ctrl_resp->is_pressed(1))
			{
				repeat_calibration = true;
				done = true;
			}

			if (m_ctrl_resp->is_pressed(2))
				{done = true;}
		}
		done = false;
	}

	// Save calibration
	save_calibration_to_config();

	// Restore the previous state of ecl_process
	if(was_sampling)
		m_daq->start_sampling();
	
	if(had_file)
	{
		m_output_file = had_file;
		m_b_output_file_registered = true;
	}
	
	for (int i=0; i<m_ctrl_regions_status.size(); i++)
		m_ctrl_regions_status[i] = reg_stat[i];

	// Calibration done
	return true;
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_process::measure_calibration_point()
//	DESCRIPTION:	Measure a single calibration point
//--------------------------------------------------------------------
bool ecl_process::measure_calibration_point(long n, ecl_response * subj_resp, ecl_response * ctrl_resp)
{
	// Should only be called by calibrate(), which has already done error checking
	
	long ctrl_x0, ctrl_y0, ctrl_w, ctrl_h, ctrl_xmid, ctrl_ymid;
	
	if (m_ctrl_mon && m_ctrl_screen_preload_h)
	{
		ctrl_x0 = m_ctrl_screen_main_area.left;
		ctrl_y0 = m_ctrl_screen_main_area.top;
		ctrl_w = m_ctrl_screen_main_area.right - m_ctrl_screen_main_area.left;
		ctrl_h = m_ctrl_screen_main_area.bottom - m_ctrl_screen_main_area.top;
		ctrl_xmid = ctrl_x0 + (ctrl_w/2);
		ctrl_ymid = ctrl_y0 + (ctrl_h/2);
	}

	long ctrl_pxx, ctrl_pxy;
	long subj_w = m_subj_mon->get_width_pix();
	long subj_h = m_subj_mon->get_height_pix();
	
	vector<long> daq_buf_x; daq_buf_x.reserve(m_config.m_cal_samples_pre_n+m_config.m_cal_samples_post_n);
	vector<long> daq_buf_y; daq_buf_y.reserve(m_config.m_cal_samples_pre_n+m_config.m_cal_samples_post_n);
	vector<bool> daq_buf_b; daq_buf_b.reserve(m_config.m_cal_samples_pre_n+m_config.m_cal_samples_post_n);
	vector<bool> daq_buf_t; daq_buf_t.reserve(m_config.m_cal_samples_pre_n+m_config.m_cal_samples_post_n);

	vector<long>::iterator itx = daq_buf_x.begin();
	vector<long>::iterator ity = daq_buf_y.begin();
	vector<bool>::iterator itb = daq_buf_b.begin();
	vector<bool>::iterator itt = daq_buf_t.begin();
	
	bool done = false;
	long t_passed;
	long daq_x, daq_y, pix_x, pix_y;
	bool blink, track;
	bool pressed = false;
	long extra_samples = 0;

	while(!done)
	{
		// Retrieve sample
		t_passed = m_daq->get_samples(m_daq_channels);
			
		if (!t_passed)
			continue;
			
		if (t_passed < 0)
		{
			ecl_error("ECL: ecl_process::calibrate()) - Could not sample daq");
			return false;
		}

		daq_x = m_daq_channels[0];
		daq_y = m_daq_channels[1];
   
		if (m_daq_channels[2] > 3072)
			{blink = true;}
		else
			{blink = false;}

		if (m_daq_channels[3] > 3072)
			{track = true;}
		else
			{track = false;}

		// Update control monitor
		if (m_ctrl_mon && m_ctrl_screen_preload_h)
		{
			// Draw the eye using the standard control screen function
			// Regions have been temporarily disabled
			ctrl_update_screen();

			// Then add the calibration data
			// Old points
			for (int i=0;i<n;i++)
			{
				ctrl_pxx = ctrl_x0 + (long)(((double)m_cal_grid_x[i]/subj_w)*(double)ctrl_w);
				ctrl_pxy = ctrl_y0 + (long)(((double)m_cal_grid_y[i]/subj_h)*(double)ctrl_h);
				m_ctrl_mon->draw_oval(m_ctrl_screen_preload_h,ctrl_pxx,ctrl_pxy,ECL_CAL_POINT_SIZE,ECL_CAL_POINT_SIZE,RGB(100,0,0));
			}
				
			// Current point
			ctrl_pxx = ctrl_x0 + (long)(((double)m_cal_grid_x[n]/subj_w)*(double)ctrl_w);
			ctrl_pxy = ctrl_y0 + (long)(((double)m_cal_grid_y[n]/subj_h)*(double)ctrl_h);
			m_ctrl_mon->draw_oval(m_ctrl_screen_preload_h,ctrl_pxx,ctrl_pxy,ECL_CAL_POINT_SIZE,ECL_CAL_POINT_SIZE,RGB(200,200,200));

			// Future points
			for (int i=n+1;i<m_cal_grid_x.size();i++)
			{
				ctrl_pxx = ctrl_x0 + (long)(((double)m_cal_grid_x[i]/subj_w)*(double)ctrl_w);
				ctrl_pxy = ctrl_y0 + (long)(((double)m_cal_grid_y[i]/subj_h)*(double)ctrl_h);

				m_ctrl_mon->draw_oval(m_ctrl_screen_preload_h,ctrl_pxx,ctrl_pxy,ECL_CAL_POINT_SIZE,ECL_CAL_POINT_SIZE,RGB(0,0,100));
			}
		
			// Previous eye positions, if a calibration is already present
			if(m_b_calibrated)
			{
				for (int i=0;i<n;i++)
				{
					pix_x=daq2pix_x(m_cal_tmp_pix_x[i]);
					pix_y=daq2pix_y(m_cal_tmp_pix_y[i]);

					ctrl_pxx = ctrl_x0 + (long)(((double)pix_x/subj_w)*(double)ctrl_w);
					ctrl_pxy = ctrl_y0 + (long)(((double)pix_y/subj_h)*(double)ctrl_h);	
					
					m_ctrl_mon->draw_oval(m_ctrl_screen_preload_h,ctrl_pxx,ctrl_pxy,ECL_CAL_POINT_SIZE,ECL_CAL_POINT_SIZE,RGB(0,50,0));
				}
			}

			// Present to the screen
			m_ctrl_mon->blit_preloaded_image(m_ctrl_screen_preload_h,m_ctrl_mon->get_width_pix()/2,m_ctrl_mon->get_height_pix()/2);
			m_ctrl_mon->present();
		}

		// Allow escape
		if(ctrl_resp->is_pressed(1))
			return false;

		// Register DAQ measurements into a cyclic buffer	
		*itx = daq_x;
		*ity = daq_y;
		*itt = track;
		*itb = blink;
		
		itx++; ity++; itt++; itb++;

		if (itx > daq_buf_x.end())
		{
			itx = daq_buf_x.begin();
			ity = daq_buf_y.begin();
			itt = daq_buf_t.begin();
			itb = daq_buf_b.begin();
		}

		// Summarize the buffer when a button is pressed
		if (subj_resp->is_pressed(0) || ctrl_resp->is_pressed(0) && (daq_buf_x.size() > m_config.m_cal_samples_pre_n))
			pressed = true;

		// But first collect some more data
		if(pressed)
			extra_samples++;

		if(extra_samples>=m_config.m_cal_samples_post_n)
			done = true;
	}
	
	// Average over all points where the signal was good
	unsigned long n_good_samples = 0;
	unsigned long sum_daq_x = 0;
	unsigned long sum_daq_y = 0;

	for (int i=0; i<daq_buf_x.size(); i++)
	{
		if(daq_buf_t[i] && !daq_buf_b[i])
		{
			n_good_samples++;
			sum_daq_x += daq_buf_x[i];
			sum_daq_y += daq_buf_y[i];
		}
	}

	m_cal_tmp_daq_x[n] = sum_daq_x/n_good_samples;
	m_cal_tmp_daq_y[n] = sum_daq_y/n_good_samples;

	if ((double)n_good_samples/(double)daq_buf_x.size() > 0.75)
		m_cal_tmp_good[n] = true;
	else
		m_cal_tmp_good[n] = false;
	// Possible other conditions: too much variation, extremely deviant

	return true;
}
//

//--------------------------------------------------------------------
//	FUNCTION:		ecl_process::drift_correction()
//	DESCRIPTION:	Correct drift by measuring a single point
//--------------------------------------------------------------------
bool ecl_process::drift_correction(long pix_x, long pix_y)
{	
	// Error checking
	if(!m_b_initialised)
    {
        ecl_error("ECL: ecl_process::drift_correction()) - Initialize ecl_process first");
        return false;
    }

	if (m_subj_mon == NULL)
	{
		ecl_error("ECL: ecl_process::drift_correction()) - Missing monitor device");
        return false;
	}

	if (!m_daq->is_sampling())
	{
		ecl_error("ECL: ecl_process::drift_correction()) - Must be sampling");
        return false;
	}


	// Take one sample
	sample();				
	ctrl_update_screen();
	m_ctrl_mon->present();

	double diff_x = abs(m_subj_mon->compute_visual_angle((double)(pix_x-get_eye_x()),(double)(m_subj_mon->get_distance_pix())));
	double diff_y = abs(m_subj_mon->compute_visual_angle((double)(pix_y-get_eye_y()),(double)(m_subj_mon->get_distance_pix())));



	// Check whether the drift correction should be counted
	bool good = false;

	if (is_in_safe_fixation() && diff_x < m_config.m_cal_drift_deg_x && diff_y < m_config.m_cal_drift_deg_y)
		good = true;

	// Adjust the calibration
	if(good)
	{
		m_config.m_cal_totdrift_pix_x +=  pix_x-get_eye_x();
		m_config.m_cal_totdrift_pix_y +=  pix_y-get_eye_y();

		m_config.m_cal_params[1] += pix_x-get_eye_x();
		m_config.m_cal_params[3] += pix_y-get_eye_y();
	}

	return good;
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_process::sample()
//	DESCRIPTION:	Fetch and process a single sample
//--------------------------------------------------------------------
bool ecl_process::sample(long tstamp)
{
    // Require initialisation
    if(!m_b_initialised)
    {
        ecl_error("ECL: ecl_process::sample()) - Initialize ecl_process first");
        return false;
    }

	// Require calibration
	if(!m_b_calibrated)
    {
        ecl_error("ECL: ecl_process::sample()) - Calibrate first");
        return false;
    }


    // Fetch the current DAQ sample
    long t_passed;
    t_passed = m_daq->get_samples(m_daq_channels);

    // If the daq clock hasn't advanced, change nothing
    if (!t_passed)
        {return true;}

    // If an error was returned from the DAQ, also return an error
    if (t_passed < 0)
    {
        ecl_error("ECL: ecl_process::sample()) - Could not sample daq");
        return false;
    }

    // Advance the ecl_process clock using the DAQ clock
    m_daq_time = m_daq_time + t_passed;

    // Fetch all tracked response button statuses
	long h;
	ecl_response *p;
	string s;

	for(unsigned int i=0;i<m_p_tracked_keys.size();i++)
	{
		h = m_h_tracked_keys[i];
		p = m_p_tracked_keys[i];
		s = m_s_tracked_keys[i];

		if(p->is_pressed(s))
			{key_down(h);}
		else
			{key_up(h);}
	}

    // Process the eye coordinates
    m_status_daq_x = m_daq_channels[0];
	m_status_daq_y = m_daq_channels[1];
    m_status_pix_x = daq2pix_x(m_status_daq_x);
	m_status_pix_y = daq2pix_y(m_status_daq_y);

    // Process the eye status
    if (t_passed > m_config.m_timeout_limit)
    {m_b_status_timeout = true;}
    else
    {m_b_status_timeout = false;}

    if (m_daq_channels[2] > 3072)
    {m_b_status_blink = true;}
    else
    {m_b_status_blink = false;}

    if (m_daq_channels[3] > 3072)
    {m_b_status_track = true;}
    else
    {m_b_status_track = false;}

    if (m_status_pix_x > m_flocktol_rect.left && m_status_pix_x < m_flocktol_rect.right && m_status_pix_y > m_flocktol_rect.top && m_status_pix_y < m_flocktol_rect.bottom)
    {m_b_status_good = true;}
    else
    {m_b_status_good = false;}

    m_b_status_saccade = false;
	m_b_status_safe = false;

    if (m_b_output_file_registered)
	{
		vector<bool> eye_status;
		eye_status.reserve(6);
		eye_status.push_back(m_b_status_saccade);
		eye_status.push_back(m_b_status_safe);
		eye_status.push_back(m_b_status_good);
		eye_status.push_back(m_b_status_blink);
		eye_status.push_back(m_b_status_track);
		eye_status.push_back(m_b_status_timeout);
		
		m_output_file->add_sample(tstamp, m_status_daq_x, m_status_daq_y, m_status_pix_x, m_status_pix_y, eye_status, m_b_status_stim, m_b_status_key, m_b_status_flag);
	}

    return true;
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_process::keep_sampling()
//	DESCRIPTION:	Blocking call to keep sampling for a given time
//--------------------------------------------------------------------
bool ecl_process::keep_sampling(long t, string mode)
{
    // Currently only implemented unbuffered sampling
    // For buffered sampling, increase t manually whenever possible

    // if (mode == UNBUFFERED)

	if(!m_b_initialised)
    {
        ecl_error("ECL: ecl_process::keep_sampling()) - Initialize ecl_process first");
        return false;
    }

    long start_t = m_daq_time;
    long current_t = m_daq_time;
    bool res = sample();

    while ((current_t - start_t) < t)
    {
        res = sample();
        current_t = m_daq_time;

        if(!res)
        {return false;}
    }

    return true;
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_process::add_stim_flag()
//	DESCRIPTION:	Add N stim flags
//--------------------------------------------------------------------
void ecl_process::add_stim_flag(long n)
{
	for (int i=0; i<n; i++)
		m_b_status_stim.push_back(false);
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_process::add_stim_flag()
//	DESCRIPTION:	Add N key flags
//--------------------------------------------------------------------
void ecl_process::add_key_flag(long n)
{
	for (int i=0; i<n; i++)
		m_b_status_key.push_back(false);
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_process::add_other_flag()
//	DESCRIPTION:	Add N other flags
//--------------------------------------------------------------------
void ecl_process::add_other_flag(long n)
{
	for (int i=0; i<n; i++)
		m_b_status_flag.push_back(false);
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_process::add_keyer_flag()
//	DESCRIPTION:	Add a key flag
//--------------------------------------------------------------------
void ecl_process::add_key_flag(ecl_response * resp_dev, string resp_button)
{
	m_s_tracked_keys.push_back(resp_button);
	m_p_tracked_keys.push_back(resp_dev);
	m_b_status_key.push_back(false);
	m_h_tracked_keys.push_back(m_b_status_key.size()-1);
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_process::construct_calibration_grid()
//	DESCRIPTION:	Position the calibration points on the screen
//--------------------------------------------------------------------
bool ecl_process::construct_calibration_grid(string dotpattern)
{
    if(!m_b_initialised)
    {
        ecl_error("ECL: ecl_process::construct_calibration_grid()) - Initialize ecl_process first");
        return false;
    }
	
	if(m_subj_mon == NULL)
    {
        ecl_error("ECL: ecl_process::construct_calibration_grid() - Subject monitor is required");
        return false;
    }

    long cal_center_x = (m_cal_rect.right - m_cal_rect.left)/2;
    long cal_center_y = (m_cal_rect.bottom - m_cal_rect.top)/2;

    if (m_cal_tmp_pix_x.size() || m_cal_tmp_pix_y.size())
    {
        m_cal_tmp_pix_x.clear();
        m_cal_tmp_pix_y.clear();

        m_cal_tmp_pix_x.reserve(m_config.m_cal_points_n);
        m_cal_tmp_pix_y.reserve(m_config.m_cal_points_n);
    }

    // Only implented for dotpattern == "DEFAULT_DIR_A"
	// I.e., diagonal cross from top-left to bottom-right, then bottom-left to top-right

	if (m_config.m_cal_points_n%2)
	{
	    ecl_error("ECL: ecl_process::construct_calibration_grid() - Even number of points required");
	    return false;
	}

	long halfway = (m_config.m_cal_points_n/2)-1;
    long x,y;
	
	m_cal_grid_x.clear();
	m_cal_grid_y.clear();
	m_cal_grid_x.reserve(m_config.m_cal_points_n);
	m_cal_grid_y.reserve(m_config.m_cal_points_n);

	for (int j=0; j<=halfway; j++)
	{
        x = (long) (m_cal_rect.left + ((m_cal_rect.right - m_cal_rect.left) * ((double)j/(double)halfway)));
        m_cal_grid_x.push_back(x);
        y = (long) (m_cal_rect.top + ((m_cal_rect.bottom - m_cal_rect.top) * ((double)j/(double)halfway)));
        m_cal_grid_y.push_back(y);
	}

	for (unsigned int j=halfway+1; j<m_config.m_cal_points_n; j++)
	{
        x = (long) (m_cal_rect.left + ((m_cal_rect.right - m_cal_rect.left) * (((double)j-(double)halfway-1)/((double)m_config.m_cal_points_n-(double)halfway-2))));
        m_cal_grid_x.push_back(x);
        y = (long) (m_cal_rect.bottom - ((m_cal_rect.bottom - m_cal_rect.top) * (((double)j-(double)halfway-1)/((double)m_config.m_cal_points_n-(double)halfway-2))));
        m_cal_grid_y.push_back(y);
	}

    return true;
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_process::add_calibration_point
//	DESCRIPTION:	Add one calibration measurement
//--------------------------------------------------------------------
bool ecl_process::add_calibration_point(long pix_x, long pix_y, long daq_x, long daq_y)
{
    if(!m_b_initialised)
    {
        ecl_error("ECL: ecl_process::add_calibration_point() - Initialize ecl_process first");
        return false;
    }
	
	if (m_cal_tmp_pix_x.size()+1 > m_config.m_cal_points_n || m_cal_tmp_pix_y.size()+1 > m_config.m_cal_points_n || m_cal_tmp_daq_x.size()+1 > m_config.m_cal_points_n || m_cal_tmp_daq_y.size()+1 > m_config.m_cal_points_n)
    {
     ecl_error("ECL: ecl_process::add_calibration_point() - Too many calibration points added");
     return false;
    }
    m_cal_tmp_pix_x.push_back(pix_x);
    m_cal_tmp_pix_y.push_back(pix_y);
    m_cal_tmp_daq_x.push_back(daq_x);
    m_cal_tmp_daq_y.push_back(daq_y);

    return true;
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_process::replace_calibration_point
//	DESCRIPTION:	Replace one calibration measurement
//--------------------------------------------------------------------
bool ecl_process::replace_calibration_point(long n, long daq_x, long daq_y)
{
    if(!m_b_initialised)
    {
        ecl_error("ECL: ecl_process::replace_calibration_point() - Initialize ecl_process first");
        return false;
    }
	
	if ((unsigned long)n > m_cal_tmp_pix_x.size() || n < 1)
    {
        ecl_error("ECL: ecl_process::replace_calibration_point() - Calibration point not found");
        return false;
    }

    m_cal_tmp_daq_x[n-1] = daq_x;
    m_cal_tmp_daq_x[n-1] = daq_y;

    return true;
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_process::fit_calibration()
//	DESCRIPTION:	Fit a model to the calibration data
//--------------------------------------------------------------------
bool ecl_process::fit_calibration()
{
    if(!m_b_initialised)
    {
        ecl_error("ECL: ecl_process::fit_calibration() - Initialize ecl_process first");
        return false;
    }
	
	unsigned long n = m_config.m_cal_points_n;

    if (m_cal_tmp_pix_x.size() < n || m_cal_tmp_pix_y.size() || m_cal_tmp_daq_x.size() || m_cal_tmp_daq_y.size())
    {
        ecl_error("ECL: ecl_process::fit_calibration() - Not enough measurements");
        return false;
    }

    if (m_cal_tmp_params.size() != 4 || m_cal_tmp_fit.size() != 2)
    {
        ecl_error("ECL: ecl_process::fit_calibration() - Params and fit vectors are not allocated properly");
        return false;
    }

    long correct_n = 0;

    unsigned long pix_x_sum = 0L;
    unsigned long pix_y_sum = 0L;
    long daq_x_sum = 0;
    long daq_y_sum = 0;
    long daq_pix_x_sum = 0;
    long daq_pix_y_sum = 0;
    long pix_xx_sum = 0;
    long pix_yy_sum = 0;
    long daq_xx_sum = 0;
    long daq_yy_sum = 0;

    double pix_x_div = 0.0;
    double pix_y_div = 0.0;
    double daq_x_div = 0.0;
    double daq_y_div = 0.0;

    for (int i=0; i < n; i++)
	{
		if (!m_cal_tmp_good[i])
            continue;

        correct_n = correct_n++;

        pix_x_sum += m_cal_tmp_pix_x[i];
        pix_y_sum += m_cal_tmp_pix_y[i];
        daq_x_sum += m_cal_tmp_daq_x[i];
        daq_y_sum += m_cal_tmp_daq_y[i];
        daq_pix_x_sum += m_cal_tmp_pix_x[i] * m_cal_tmp_daq_x[i];
        daq_pix_y_sum += m_cal_tmp_pix_y[i] * m_cal_tmp_daq_y[i];
        pix_xx_sum += m_cal_tmp_pix_x[i]*m_cal_tmp_pix_x[i];
        pix_yy_sum += m_cal_tmp_pix_y[i]*m_cal_tmp_pix_y[i];
        daq_xx_sum += m_cal_tmp_daq_x[i]*m_cal_tmp_daq_x[i];
        daq_yy_sum += m_cal_tmp_daq_y[i]*m_cal_tmp_daq_y[i];
	}

	if (correct_n<4)
    {
        ecl_error("ECL: ecl_process::fit_calibration() - We want at least 4 good measurements");
        return false;
    }

	pix_x_div = (double) (n * (double) pix_xx_sum - (pix_x_sum * (double) pix_x_sum));
	daq_x_div = (double) (n * (double) daq_xx_sum - (daq_x_sum * (double) daq_x_sum));
	pix_y_div = (double) (n * (double) pix_yy_sum - (pix_y_sum * (double) pix_y_sum));
	daq_y_div = (double) (n * (double) daq_yy_sum - (daq_y_sum * (double) daq_y_sum));

	if (!pix_x_div || !daq_x_div)
	{
	    m_cal_tmp_params[0] = 0;
	    m_cal_tmp_params[1] = 0;
	    m_cal_tmp_fit[0] = 0;
    }
	else
	{
		 m_cal_tmp_params[0] = ((double) (              n * daq_pix_x_sum - (double) pix_x_sum * daq_x_sum )) / pix_x_div;
		 m_cal_tmp_params[1] = ((double) ((double) pix_xx_sum * daq_x_sum  - (double) pix_x_sum * daq_pix_x_sum)) / pix_x_div;

		// Calculate correlation coefficient
		m_cal_tmp_fit[0] = ((double) (n * daq_pix_x_sum - (double) pix_x_sum * daq_x_sum)) / sqrt (pix_x_div * daq_x_div);
	}

    if (!pix_y_div || !daq_y_div)
	{
	    m_cal_tmp_params[2] = 0;
	    m_cal_tmp_params[3] = 0;
	    m_cal_tmp_fit[1] = 0;
    }
	else
	{
		 m_cal_tmp_params[2] = ((double) (              n * daq_pix_y_sum - (double) pix_y_sum * daq_y_sum )) / pix_y_div;
		 m_cal_tmp_params[3] = ((double) ((double) pix_yy_sum * daq_y_sum  - (double) pix_y_sum * daq_pix_y_sum)) / pix_y_div;

		// Calculate correlation coefficient
		m_cal_tmp_fit[1] = ((double) (n * daq_pix_y_sum - (double) pix_y_sum * daq_y_sum)) / sqrt (pix_y_div * daq_y_div);
	}
	
	for (int i=0; i<n; i++)
	{
		m_cal_tmp_pix_x[i] = (long)(((double)m_cal_tmp_daq_x[i]*m_cal_tmp_params[0]) + m_cal_tmp_params[1]);
		m_cal_tmp_pix_y[i] = (long)(((double)m_cal_tmp_daq_y[i]*m_cal_tmp_params[0]) + m_cal_tmp_params[1]);
	}

	return true;
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_process::save_calibration_to_config()
//	DESCRIPTION:	Store and use the calibration values
//--------------------------------------------------------------------
bool ecl_process::save_calibration_to_config()
{
    if(!m_b_initialised)
    {
        ecl_error("ECL: ecl_process::save_calibration_to_config() - Initialize ecl_process first");
        return false;
    }
	
	long n = m_config.m_cal_points_n;

    if (m_cal_tmp_pix_x.size() != n || m_cal_tmp_pix_y.size() != n || m_cal_tmp_daq_x.size() != n || m_cal_tmp_daq_y.size() != n)
    {
        ecl_error("ECL: ecl_process::save_calibration_to_config() - Incorrect number of measurements");
        return false;
    }

    if (m_cal_tmp_params.size() != m_config.m_cal_params.size() || m_cal_tmp_fit.size() != m_config.m_cal_fit.size())
    {
        ecl_error("ECL: ecl_process::fit_calibration() - Params and fit vectors are of incorrect size");
        return false;
    }

    if (!m_cal_tmp_params[0] || !m_cal_tmp_params[2])
    {
        ecl_error("ECL: ecl_process::save_calibration_to_config() - Invalid calibration, linear weights must be non-zero");
        return false;
    }

    m_config.m_cal_daqvals_x = m_cal_tmp_daq_x;
    m_config.m_cal_daqvals_y = m_cal_tmp_daq_y;
    m_config.m_cal_pixvals_x = m_cal_tmp_pix_x;
    m_config.m_cal_pixvals_y = m_cal_tmp_pix_y;

    m_config.m_cal_params = m_cal_tmp_params;
    m_config.m_cal_fit = m_cal_tmp_fit;
	time(&m_config.m_cal_time);

	for (int i=0; i<n; i++)	
	{
		m_cal_tmp_pix_x[i] = 0;
		m_cal_tmp_pix_y[i] = 0;
		m_cal_tmp_daq_x[i] = 0;
		m_cal_tmp_daq_y[i] = 0;
		m_cal_tmp_good[i] = false;
	}

	for (int i=0; i<m_config.m_cal_params.size(); i++)	
		m_cal_tmp_params[i] = 0.0;

	for (int i=0; i<m_config.m_cal_fit.size(); i++)	
		m_cal_tmp_fit[i] = 0.0;

    m_b_calibrated = true;
    return true;
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_process::daq2pix_x()
//	DESCRIPTION:	Convert DAQ to pixel values in the X dimension
//--------------------------------------------------------------------
long ecl_process::daq2pix_x(long daq_x)
{
    if(!m_b_calibrated)
    {
        ecl_error("ECL: ecl_process::daq2pix_x() - Calibrate first");
        return 0;
    }

    long pix_x = (long)(((double)daq_x*m_config.m_cal_params[0]) + m_config.m_cal_params[1]);

    return pix_x;
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_process::daq2pix_y()
//	DESCRIPTION:	Convert DAQ to pixel values in the Y dimension
//--------------------------------------------------------------------
long ecl_process::daq2pix_y(long daq_y)
{
    if(!m_b_calibrated)
    {
        ecl_error("ECL: ecl_process::daq2pix_x() - Calibrate first");
        return 0;
    }

    long pix_y = (long)(((double)daq_y*m_config.m_cal_params[2]) + m_config.m_cal_params[3]);

    return pix_y;
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_process::pix2daq_x()
//	DESCRIPTION:	Convert pixel to DAQ values in the X dimension
//--------------------------------------------------------------------
long ecl_process::pix2daq_x(long pix_x)
{
    if(!m_b_calibrated)
    {
        ecl_error("ECL: ecl_process::pix2daq_x() - Calibrate first");
        return 0;
    }

    long daq_x = (long)(((double)pix_x - m_config.m_cal_params[1])/m_config.m_cal_params[0]);

    return daq_x;
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_process::pix2daq_y()
//	DESCRIPTION:	Convert pixel to DAQ values in the Y dimension
//--------------------------------------------------------------------
long ecl_process::pix2daq_y(long pix_y)
{
    if(!m_b_calibrated)
    {
        ecl_error("ECL: ecl_process::pix2daq_y() - Calibrate first");
        return 0;
    }

    long daq_y = (long)(((double)pix_y - m_config.m_cal_params[3])/m_config.m_cal_params[2]);

    return daq_y;
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_process::attach_output_file()
//	DESCRIPTION:	Register file for automatic output
//--------------------------------------------------------------------
bool ecl_process::attach_output_file(ecl_output & output_file)
{
	if(!m_b_initialised)
    {
        ecl_error("ECL: ecl_process::attach_output_file() - Initialize ecl_process first");
        return false;
    }
	
	if (m_b_output_file_registered)
	{
		ecl_error("ECL: ecl_process::attach_output_file() - Output file has already been registered");
        return false;
	}

	if(!output_file.has_buffer())
	{
		ecl_error("ECL: ecl_process::attach_output_file() - Output file has no buffer");
        return false;
	}

	m_output_file = &output_file;
	
	m_b_output_file_registered = true;

	return true;
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_process::unregister_output_file()
//	DESCRIPTION:	Unregister file for automatic output
//--------------------------------------------------------------------
bool ecl_process::detach_output_file()
{
	if (!m_b_output_file_registered)
	{
		ecl_error("ECL: ecl_process::detach_output_file() - No output file registered");
        return false;
	}

	// m_output_file->save_buffer(&m_config);
	// m_output_file->clear_buffer();

	m_output_file = NULL;
	m_b_output_file_registered = false;

	return true;
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_process::save_output_file()
//	DESCRIPTION:	Save buffer to file
//--------------------------------------------------------------------
bool ecl_process::save_output_file()
{
	if (!m_b_output_file_registered)
	{
		ecl_error("ECL: ecl_process::save_output_file() - No output file registered");
        return false;
	}

	bool res = m_output_file->save_buffer(&m_config);
	clear_output_buffer();

	return res;
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_process::clear output buffer()
//	DESCRIPTION:	Clear the output buffer
//--------------------------------------------------------------------
bool ecl_process::clear_output_buffer()
{
	if (!m_b_output_file_registered)
	{
		ecl_error("ECL: ecl_process::clear_output_buffer() - No output file registered");
        return false;
	}

	return m_output_file->clear_buffer();
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_process::add_to_msgbuf()
//	DESCRIPTION:	Add a line to the message buffer
//--------------------------------------------------------------------
bool ecl_process::add_to_msgbuf(string msg)
{
	if (msg.size() > ECL_MSGBUF_MAXL)
	{
		ecl_error("ECL: ecl_process::add_to_msgbuf() - Message is too long");
        return false;
	}
	
	m_msg_buf.push_back(msg);
	return true;
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_process::add_to_msgbuf()
//	DESCRIPTION:	After maximal size has been reached, remove the first item
//--------------------------------------------------------------------
bool ecl_process::add_to_msgbuf(string msg, long n)
{
	if (!add_to_msgbuf(msg))
		{return false;}

	return prune_msgbuf(n);
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_process::get_from_msgbuf()
//	DESCRIPTION:	Fetch one line from the message buffer
//--------------------------------------------------------------------
string ecl_process::get_from_msgbuf(unsigned long n)
{
	if (n > m_msg_buf.size() || n < 1)
	{
		ecl_error("ECL: ecl_process::get_from_msgbuf() - Invalid line number");
        return "";
	}

	return m_msg_buf[n-1];
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_process::clear_msgbuf()
//	DESCRIPTION:	Empty the message buffer
//--------------------------------------------------------------------
bool ecl_process::clear_msgbuf()
{
	m_msg_buf.clear();
	return true;
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_process::prune_msgbuf()
//	DESCRIPTION:	Prune the message buffer, dropping the oldest items
//--------------------------------------------------------------------
bool ecl_process::prune_msgbuf(unsigned long n)
{
	if (n > m_msg_buf.size())
	{
		return true;
	}

	if (n < 1)
	{
		ecl_error("ECL: ecl_process::prune_msgbuf() - Invalid argument");
        return false;
	}

	m_msg_buf.erase(m_msg_buf.begin(),m_msg_buf.end()-n);

	return true;
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_process::draw_msgbuf()
//	DESCRIPTION:	Draw textbuffer to the screen
//--------------------------------------------------------------------
bool ecl_process::draw_msgbuf(long x0, long y0, long h, long w, ecl_LUTc fg_c, ecl_LUTc bg_c, ecl_monitor * mon)
{
	bool res = true;

	if (!m_msg_buf.size())
	{
		ecl_error("ECL: ecl_process::draw_msgbuf() - Text buffer is empty");
		return false;
	}

	if (mon == NULL)
	{
		if (m_ctrl_mon == NULL)
		{
			ecl_error("ECL: ecl_process::draw_msgbuf() - No valid monitor");
			return false;
		}
		else
		{
			mon = m_ctrl_mon;
		}
	}

	unsigned long handle = mon->create_preloaded_image(w,h);
	if (!mon->is_valid_preloaded_image(handle))
	{
		ecl_error("ECL: ecl_process::draw_msgbuf() - Could not draw text buffer");
		return false;
	}

	res = res && mon->draw_rect(handle, w/2, h/2, w, h, bg_c);

	long line_height = mon->get_font_size();

	if (line_height < 1 || line_height > h)
	{
		ecl_error("ECL: ecl_process::draw_msgbuf() - Invalid font setting");
		return false;
	}

	unsigned long max_line = h/line_height;
	if (m_msg_buf.size() < max_line)
	{
		max_line = m_msg_buf.size();
	}

	for (unsigned int i=0; i<max_line; i++)
	{
		res = res && mon->draw_text(handle, m_msg_buf[i], line_height + x0, y0+(i*line_height), fg_c);
	}

	res = res && mon->blit_preloaded_image(handle, x0 + w/2, y0 + h/2);

	if (!res)
	{
		ecl_error("ECL: ecl_process::draw_msgbuf() - An error occurred when drawing the text buffer");
		return false;
	}

	return true;
}	
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_process::add_region()
//	DESCRIPTION:	Add a region to ecl_process
//--------------------------------------------------------------------
unsigned long ecl_process::add_region(ecl_region * reg_p, string reg_name)
{
	if(!m_b_initialised)
    {
        ecl_error("ECL: ecl_process::add_region() - Initialize ecl_process first");
        return 0;
    }

	if (!reg_p->is_set())
	{
		ecl_error("ECL: ecl_process::add_region() - Region has not been set");
        return 0;
	}

	m_regions_pointer.push_back(reg_p);
	m_regions_name.push_back(reg_name);
	m_ctrl_regions_status.push_back(1);

	if (m_ctrl_mon != NULL)
	{
		double scale = ((double)m_subj_mon->get_height_pix())/((double)m_ctrl_screen_main_area.bottom - (double)m_ctrl_screen_main_area.top);
		reg_p->preload_image(m_ctrl_mon,RGB(200,200,200),RGB(100,100,100),scale);
	}

	if (m_regions_pointer.size() != m_regions_name.size() || m_regions_pointer.size() != m_ctrl_regions_status.size())
	{
		ecl_error("ECL: ecl_process::add_region() - Something went wrong");
        return 0;
	}

	return m_regions_pointer.size();
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_process::get_region_pointer()
//	DESCRIPTION:	Retrieve the region pointer
//--------------------------------------------------------------------
ecl_region * ecl_process::get_region_pointer(unsigned long handle)
{
	if(!m_b_initialised)
    {
        ecl_error("ECL: ecl_process::get_region_pointer() - Initialize ecl_process first");
        return NULL;
    }

	if(handle > m_regions_pointer.size())
    {
        ecl_error("ECL: ecl_process::get_region_pointer() - Invalid handle");
        return NULL;
    }

	ecl_region * p = m_regions_pointer[handle-1];

	if(p==NULL)
	{
		ecl_error("ECL: ecl_process::get_region_pointer() - Returning NULL pointer!");
	}

	return p;
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_process::get_region_pointer()
//	DESCRIPTION:	Retrieve the region pointer
//--------------------------------------------------------------------
ecl_region * ecl_process::get_region_pointer(string reg_name)
{
	if(!m_b_initialised)
    {
        ecl_error("ECL: ecl_process::get_region_pointer() - Initialize ecl_process first");
        return 0;
    }

	unsigned long handle = get_region_handle(reg_name);

	return get_region_pointer(handle);
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_process::get_region_handle()
//	DESCRIPTION:	Retrieve the region handle
//--------------------------------------------------------------------
unsigned long ecl_process::get_region_handle(string reg_name)
{
	if(!m_b_initialised)
    {
        ecl_error("ECL: ecl_process::get_region_handle() - Initialize ecl_process first");
        return 0;
    }

	vector<string>::iterator it;
	unsigned long handle = 0;

	for (it=m_regions_name.begin(); it < m_regions_name.end(); it++)
	{
		handle++;
		if (*it == reg_name)
		{
			return handle;
		}
	}

	ecl_error("ECL: ecl_process::get_region_handle() - Name not found");
	return 0;
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_process::get_region_name()
//	DESCRIPTION:	Retrieve the region name
//--------------------------------------------------------------------
string ecl_process::get_region_name(unsigned long handle)
{
	if(!m_b_initialised)
    {
        ecl_error("ECL: ecl_process::get_region_name() - Initialize ecl_process first");
        return "";
    }

	if(handle > m_regions_name.size())
    {
        ecl_error("ECL: ecl_process::get_region_name() - Invalid handle");
        return "";
    }

	return m_regions_name[handle-1];
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_process::clear_all_regions()
//	DESCRIPTION:	Remove all regions from ecl_process
//--------------------------------------------------------------------
bool ecl_process::clear_all_regions()
{
	if(!m_b_initialised)
    {
        ecl_error("ECL: ecl_process::clear_all_regions() - Initialize ecl_process first");
        return false;
    }

	m_regions_pointer.clear();
	m_regions_name.clear();

	return true;
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_process::on_region()
//	DESCRIPTION:	Determines whether the eye is inside a region
//--------------------------------------------------------------------
bool ecl_process::on_region(unsigned long handle)
{
	if(!m_b_initialised)
    {
        ecl_error("ECL: ecl_process::on_region() - Initialize ecl_process first");
        return false;
    }

	if(!m_b_calibrated)
    {
        ecl_error("ECL: ecl_process::on_region() - No calibration present");
        return false;
    }

	ecl_region * p = get_region_pointer(handle);

	if(p==NULL)
	{
		ecl_error("ECL: ecl_process::on_region() - Invalid region");
        return false;
	}

	return p->in_region(get_eye_x(),get_eye_y());
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_process::on_region()
//	DESCRIPTION:	Determines whether a given pixel position is inside the region
//--------------------------------------------------------------------
bool ecl_process::on_region(string reg_name)
{
	if(!m_b_initialised)
    {
        ecl_error("ECL: ecl_process::on_region() - Initialize ecl_process first");
        return false;
    }

	unsigned long handle = get_region_handle(reg_name);
	return on_region(handle);
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_process::find_region()
//	DESCRIPTION:	Return the handle to the first region the eye is in
//--------------------------------------------------------------------
unsigned long ecl_process::find_region()
{
	if(!m_b_initialised)
    {
        ecl_error("ECL: ecl_process::on_region() - Initialize ecl_process first");
        return false;
    }

	vector<ecl_region *>::iterator it;
	unsigned long handle = 0;

	for (it=m_regions_pointer.begin(); it < m_regions_pointer.end(); it++)
	{
		handle++;
		if(on_region(handle))
		{
			return handle;
		}
	}

	return 0;
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_process::ctrl_enable_region()
//	DESCRIPTION:	Turn on automatic region drawing
//--------------------------------------------------------------------
bool ecl_process::ctrl_enable_region(unsigned long handle)
{
	if(!m_b_initialised)
    {
        ecl_error("ECL: ecl_process::ctrl_enable_region() - Initialize ecl_process first");
        return false;
    }

	if(m_ctrl_mon == NULL)
    {
        ecl_error("ECL: ecl_process::ctrl_enable_region() - No control monitor specified");
        return false;
    }

	if(handle > m_ctrl_regions_status.size())
    {
        ecl_error("ECL: ecl_process::ctrl_enable_region() - Invalid handle");
        return false;
    }

	if(!m_ctrl_regions_status[handle])
	{
		m_ctrl_regions_status[handle] = 1;
	}

	return true;
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_process::ctrl_disable_region()
//	DESCRIPTION:	Turn off automatic region drawing
//--------------------------------------------------------------------
bool ecl_process::ctrl_disable_region(unsigned long handle)
{
	if(!m_b_initialised)
    {
        ecl_error("ECL: ecl_process::ctrl_disable_region() - Initialize ecl_process first");
        return false;
    }

	if(m_ctrl_mon == NULL)
    {
        ecl_error("ECL: ecl_process::ctrl_disable_region() - No control monitor specified");
        return false;
    }

	if(handle > m_ctrl_regions_status.size())
    {
        ecl_error("ECL: ecl_process::ctrl_disable_region() - Invalid handle");
        return false;
    }

	m_ctrl_regions_status[handle] = 0;

	return true;
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_process::ctrl_enable_region()
//	DESCRIPTION:	Turn on automatic region drawing
//--------------------------------------------------------------------
bool ecl_process::ctrl_enable_region(string reg_name)
{
	if(!m_b_initialised)
    {
        ecl_error("ECL: ecl_process::ctrl_enable_region() - Initialize ecl_process first");
        return false;
    }

	long handle = get_region_handle(reg_name);

	return ctrl_enable_region(handle);
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_process::ctrl_disable_region()
//	DESCRIPTION:	Turn off automatic region drawing
//--------------------------------------------------------------------
bool ecl_process::ctrl_disable_region(string reg_name)
{
	if(!m_b_initialised)
    {
        ecl_error("ECL: ecl_process::ctrl_disable_region() - Initialize ecl_process first");
        return false;
    }

	long handle = get_region_handle(reg_name);

	return ctrl_disable_region(handle);
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_process::ctrl_draw_eye()
//	DESCRIPTION:	Draw the eye
//--------------------------------------------------------------------
bool ecl_process::ctrl_draw_eye(long x, long y)
{
	if(!m_b_initialised)
    {
        ecl_error("ECL: ecl_process::ctrl_draw_eye() - Initialize ecl_process first");
        return false;
    }

	if(m_ctrl_mon == NULL)
    {
        ecl_error("ECL: ecl_process::ctrl_draw_eye() - No control monitor specified");
        return false;
    }

	if(!m_b_calibrated)
    {
        ecl_error("ECL: ecl_process::ctrl_draw_eye() - No calibration present");
        return false;
    }

	long w = m_ctrl_mon->get_width_pix();
	long h = m_ctrl_mon->get_height_pix();

	if (x<1 || x>w || y<1 || y>h)
	{
		return true; // Do nothing, not necessarily a problem
	}

	if(!m_ctrl_mon->is_valid_preloaded_image(m_ctrl_eye_preload_h))
	{
        ecl_error("ECL: ecl_process::ctrl_draw_eye() - No valid image preloaded");
        return false;
    }

	return m_ctrl_mon->blit_preloaded_image(m_ctrl_eye_preload_h,get_eye_x(),get_eye_y());
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_process::ctrl_initialise()
//	DESCRIPTION:	Set up the control screen
//--------------------------------------------------------------------
bool ecl_process::ctrl_initialise()
{
	// Check for errors
	if(m_ctrl_mon == NULL)
    {
        ecl_error("ECL: ecl_process::ctrl_initialise() - No control monitor specified");
        return false;
    }

	if(m_subj_mon == NULL)
    {
        ecl_error("ECL: ecl_process::ctrl_initialise() - No subject monitor specified");
        return false;
    }
	
	bool res = true;

	// Determine correct size of regions and store them
	double sw = (double)m_subj_mon->get_width_pix();
	double sh = (double)m_subj_mon->get_height_pix();
	double cw = (double)m_ctrl_mon->get_width_pix();
	double ch = (double)m_ctrl_mon->get_height_pix();
	double scale = 1.0;

	if (sw/sh > cw/ch)
		{scale = (cw/sw)*ECL_CTRL_SCALE_H;}
	else
		{scale = (ch/sh)*ECL_CTRL_SCALE_H;}

	m_ctrl_screen_main_area.left = ECL_CTRL_BORDER;
	m_ctrl_screen_main_area.top = ECL_CTRL_BORDER;
	m_ctrl_screen_main_area.right = ECL_CTRL_BORDER + (long)(sw*scale);
	m_ctrl_screen_main_area.bottom = ECL_CTRL_BORDER + (long)(sh*scale);

	m_ctrl_screen_msg_area.left = ECL_CTRL_BORDER;
	m_ctrl_screen_msg_area.top = (2*ECL_CTRL_BORDER) + (long)(sh*scale);
	m_ctrl_screen_msg_area.right = ECL_CTRL_BORDER + (long)(sw*scale);
	m_ctrl_screen_msg_area.bottom = (long)ch-ECL_CTRL_BORDER;

	m_ctrl_screen_info_area.left = (2*ECL_CTRL_BORDER) + (long)(sw*scale);
	m_ctrl_screen_info_area.top = ECL_CTRL_BORDER;
	m_ctrl_screen_info_area.right = (long)cw - ECL_CTRL_BORDER;
	m_ctrl_screen_info_area.bottom = ECL_CTRL_BORDER + (long)(sh*scale);

	// Set up control screen
	m_ctrl_mon->set_font("ARIAL",12);
	m_ctrl_mon->set_bg_color(RGB(0,0,0));
	m_ctrl_mon->set_fg_color(RGB(200,200,200));

	// Prepare screen image
	m_ctrl_screen_preload_h = m_ctrl_mon->create_preloaded_image((long)cw,(long)ch);
	res = res && m_ctrl_screen_preload_h;

	// Blank first to background color
	m_ctrl_mon->draw_rect(m_ctrl_screen_preload_h,(long)cw/2,(long)ch/2,(long)cw,(long)ch);

	// Put on the current version and the author 
	RECT v_ar;
	v_ar.left = (2*ECL_CTRL_BORDER) + (long)(sw*scale);
	v_ar.right = (long)cw-ECL_CTRL_BORDER;
	v_ar.top = (2*ECL_CTRL_BORDER) + (long)(sh*scale);
	v_ar.bottom = (long)ch-ECL_CTRL_BORDER;

	res = res && m_ctrl_mon->draw_text(m_ctrl_screen_preload_h,ECL_CONFIG_VERSION,v_ar.left+5,v_ar.top+5,RGB(100,0,0));

	// Put on some decorations
	ecl_LUTc c = RGB(0,100,0);
	
	res = res && m_ctrl_mon->draw_rect(m_ctrl_screen_preload_h,(long)cw/2,(long)ch/2,(long)cw,(long)ch,c,ECL_CTRL_BORDER);

	long bcenter = m_ctrl_screen_msg_area.top + (m_ctrl_screen_msg_area.bottom - m_ctrl_screen_msg_area.top)/2;
	long bsize =  (m_ctrl_screen_msg_area.bottom - m_ctrl_screen_msg_area.top) + 2*ECL_CTRL_BORDER;

	res = res && m_ctrl_mon->draw_rect(m_ctrl_screen_preload_h,(long)cw/2,bcenter,(long)cw,bsize,c,ECL_CTRL_BORDER);

	bcenter = m_ctrl_screen_info_area.left + (m_ctrl_screen_msg_area.right - m_ctrl_screen_msg_area.left)/2;
	bsize =  (m_ctrl_screen_info_area.right - m_ctrl_screen_msg_area.left) + 2*ECL_CTRL_BORDER;

	res = res && m_ctrl_mon->draw_rect(m_ctrl_screen_preload_h,bcenter,(long)ch/2,bsize,(long)ch,c,ECL_CTRL_BORDER);

	// Prepare region images
	// Happens upon adding them to ecl_process

	// Prepare eye image
	m_ctrl_eye_preload_h = m_ctrl_mon->create_preloaded_image(5,5);
	res = res && m_ctrl_eye_preload_h;
	res = res && m_ctrl_mon->draw_oval(m_ctrl_eye_preload_h,3,3,2,2,RGB(0,0,255));

	// Check if there have been any errors
	if (!res)
	{
		ecl_error("ECL: ecl_process::ctrl_initialise() - Something went wrong");
		return false;
	}

	return true;
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_process::ctrl_update_screen()
//	DESCRIPTION:	Automatically generate a control screen
//--------------------------------------------------------------------
bool ecl_process::ctrl_update_screen()
{
	if(!m_b_initialised)
    {
        ecl_error("ECL: ecl_process::ctrl_update_screen() - Initialize ecl_process first");
        return false;
    }

	if(m_ctrl_mon == NULL)
    {
        ecl_error("ECL: ecl_process::ctrl_update_screen() - No control monitor specified");
        return false;
    }

	if(m_subj_mon == NULL)
    {
        ecl_error("ECL: ecl_process::ctrl_update_screen() - No subject monitor specified");
        return false;
    }

	if(m_ctrl_last_update_t + (1000/m_ctrl_update_freq) > m_daq_time)
	{
		return true; // Do not redraw yet
	}

	bool res = true;


	// Clear the eyemov area, always
	RECT r = m_ctrl_screen_main_area;
	long w = r.right-r.left;
	long h = r.bottom-r.top;
	long hdl = m_ctrl_screen_preload_h;

//	res = res && m_ctrl_mon->clear_rect(hdl, r.left+(w/2), r.top+(h/2),w,h);


	// Blit the active regions, if sampling
	long center_x, center_y;
	long center_x_sc, center_y_sc;

	if(m_daq->is_sampling())
	{
		for (unsigned int i=0; i<m_regions_pointer.size(); i++)
		{
			if (m_ctrl_regions_status[i])
			{
				center_x = m_regions_pointer[i]->get_center_x();
				center_x_sc = r.left + (long)(((double)center_x/(double)m_subj_mon->get_width_pix())*(double)w);
				center_y = m_regions_pointer[i]->get_center_y();
				center_y_sc = r.top + (long)(((double)center_y/(double)m_subj_mon->get_height_pix())*(double)h);

				res = res && m_regions_pointer[i]->draw(get_eye_x(),get_eye_y(),center_x_sc,center_y_sc);
			}
		}
	}
	
	// Blit the eye if sampling
	if(m_daq->is_sampling())
	{
		center_x = get_eye_x();
		center_x_sc = r.left + (long)(((double)center_x/(double)m_subj_mon->get_width_pix())*(double)w);
		center_y = get_eye_y();
		center_y_sc = r.top + (long)(((double)center_y/(double)m_subj_mon->get_height_pix())*(double)h);
		
		res = res && ctrl_draw_eye(center_x_sc, center_y_sc);
	}
	
	// Blit the status indicator, always
	// res = res && ctrl_draw_status(r.left+(w/2),r.top);	
	
	// Blit the scrolling message buffer to the bottom of the screen, when not sampling
	r = m_ctrl_screen_msg_area;

	if(!m_daq->is_sampling())
	{
		res = res && draw_msgbuf(r.left, r.top, r.right-r.left,r.bottom-r.top);
	}

	// Blit trial info to the right of the screen, when not sampling
	r = m_ctrl_screen_info_area;
	long fsize = m_ctrl_mon->get_font_size();
	long ypos = r.top;

	if(!m_daq->is_sampling())
	{
		for (unsigned int i=0; i<m_ctrl_info.size(); i++)
		{
			ypos = ypos + 2*(fsize);
			res = res && m_ctrl_mon->draw_text(0,m_ctrl_info[i],r.left+fsize,ypos);
		}
	}
	
	// Blit all this to the backbuffer
	res = res && m_ctrl_mon->blit_preloaded_image(m_ctrl_screen_preload_h,m_ctrl_mon->get_width_pix()/2,m_ctrl_mon->get_height_pix()/2);
	
	if (!res)
	{
		ecl_error("ECL: ecl_process::ctrl_update_screen() - Something went wrong");
		return false;
	}

	return true;
}
//

//bool ctrl_draw_status(long x, long y);