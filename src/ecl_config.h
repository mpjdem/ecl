#ifndef _ECL_CONFIG_INCLUDED_
#define _ECL_CONFIG_INCLUDED_

//#pragma warning(disable: 4786)

#include <vector>
#include <string>
#include <ctime>

using namespace std;

#define ECL_CONFIG_VERSION "STDDPI-1.0"
#define ECL_CAL_FIT_N 2
#define ECL_CAL_PARAMS_N 4


//--------------------------------------------------------------------
//  CLASS:			ecl_config
//	DESCRIPTION:	ECL configuration class
//--------------------------------------------------------------------
class ecl_config
{
public:
	

															ecl_config();
	virtual												  ~ ecl_config();

	inline	bool											is_initialised(){return m_b_initialised;}
	

			bool											save(string fname);
			bool											load(string fname);


protected:
	friend class ecl_process;
	friend class ecl_output_pfile;

			string											m_cfg_id;

			unsigned long									m_cal_points_n;
			long											m_cal_samples_pre_n;
			long											m_cal_samples_post_n;
			double											m_cal_range_deg_x;
			double											m_cal_range_deg_y;
			double											m_cal_center_offset_deg_x;		//	0.0 = middle of the screen
			double											m_cal_center_offset_deg_y;

			double											m_cal_drift_deg_x;
			double											m_cal_drift_deg_y;

			double											m_region_tol_deg_x;
			double											m_region_tol_deg_y;

			long											m_timeout_limit;

			vector<long>									m_cal_daqvals_x, m_cal_daqvals_y;
			vector<long>									m_cal_pixvals_x, m_cal_pixvals_y; 
			double											m_cal_flocktol_deg_x;
			double											m_cal_flocktol_deg_y;

			bool											m_sac_detect;
			long											m_sac_scale_nominator; 
			long											m_sac_lower_bound;
			long											m_sac_upper_bound;
			long											m_sac_safe_velocity;
			long											m_sac_safe_duration;
			
			double											m_fix_safe_deg;

			long											m_cal_totdrift_pix_x;
			long											m_cal_totdrift_pix_y;
			vector<double>									m_cal_params;
			vector<double>									m_cal_fit;

			time_t											m_cal_time;
			bool											m_b_initialised;
};
//
#endif	//_ECL_CONFIG_INCLUDED_