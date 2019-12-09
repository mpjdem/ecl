//#pragma warning(disable: 4786)

#include "stdafx.h"
#include <cstdio>
#include <new>

#include "ecl_config.h"
#include "ecl.h"

using namespace std;


//--------------------------------------------------------------------
//	FUNCTION:		ecl_config::ecl_config()
//	DESCRIPTION:	Constructor
//--------------------------------------------------------------------
ecl_config::ecl_config()
{
	// Declared in base class
	m_cfg_id = ECL_CONFIG_VERSION; 
	m_cal_points_n = 10;
	m_cal_samples_pre_n = 150;
	m_cal_samples_post_n = 50;
	m_cal_center_offset_deg_x = 0.0;
	m_cal_center_offset_deg_y = 0.0;
	m_cal_range_deg_x = 10.0;
	m_cal_range_deg_y = 8.0;

	m_cal_drift_deg_x = 1.0;
	m_cal_drift_deg_y = 1.0;

	m_region_tol_deg_x = 0.5;
	m_region_tol_deg_y = 0.5;

	m_timeout_limit = 1;

   	m_cal_daqvals_x.clear();
	m_cal_daqvals_y.clear();
	m_cal_pixvals_x.clear();
	m_cal_pixvals_y.clear();

	/*m_cal_daqvals_x = new (nothrow) long[m_cal_points_n];
	m_cal_daqvals_y = new (nothrow) long[m_cal_points_n];
	m_cal_pixvals_x = new (nothrow) long[m_cal_points_n];
	m_cal_pixvals_y = new (nothrow) long[m_cal_points_n];
	
	if (m_cal_daqvals_x==NULL || m_cal_daqvals_y==NULL || m_cal_pixvals_x == NULL || m_cal_pixvals_y == NULL)
	{
		ecl_error("Cannot allocate memory!");
	}

	for (int i=0; i <m_cal_points_n; i++)
	{
		m_cal_daqvals_x[i] = 0;
		m_cal_daqvals_y[i] = 0;
		m_cal_pixvals_x[i] = 0;
		m_cal_pixvals_y[i] = 0;
	}*/

	// Own declarations
	m_cal_flocktol_deg_x = 1;
	m_cal_flocktol_deg_y = 1;

	m_sac_detect = true;
	m_sac_scale_nominator = 50; 
	m_sac_lower_bound = 4;
	m_sac_upper_bound = 18;
	m_sac_safe_velocity = 50;	//deg/s
	m_sac_safe_duration = 5;	//ms
			
	m_fix_safe_deg = 0.5;

	m_cal_totdrift_pix_x = 0;
	m_cal_totdrift_pix_y =0;
	m_cal_params.clear();
	m_cal_fit.clear();	


	
	m_b_initialised = false;

	/*m_cal_daqpix_corr_x = 1.0;
	m_cal_daqpix_corr_y = 1.0;
	m_cal_interc_x = 0.0;
	m_cal_interc_y = 0.0;
	m_cal_weight_x = 1.0;
	m_cal_weight_y = 1.0;*/
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_config::~ecl_config()
//	DESCRIPTION:	Deconstructor
//--------------------------------------------------------------------
ecl_config::~ecl_config()
{
   	m_cal_daqvals_x.clear();
	m_cal_daqvals_y.clear();
	m_cal_pixvals_x.clear();
	m_cal_pixvals_y.clear();

	m_cal_params.clear();
	m_cal_fit.clear();

	/*delete [] m_cal_daqvals_x;
	delete [] m_cal_daqvals_y;
	delete [] m_cal_pixvals_x;
	delete [] m_cal_pixvals_y;*/
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_config::save()
//	DESCRIPTION:	Save config to a file
//--------------------------------------------------------------------
bool ecl_config::save(string fname)
{
	vector<double>::iterator it;

	FILE * cfg_file = fopen(fname.c_str(), "w+");

	fprintf(cfg_file,"Config ID: %s\n\n",m_cfg_id.c_str());
	
	fprintf(cfg_file,"Sac detect: %d\n",(long)m_sac_detect);
	fprintf(cfg_file,"Sac boundaries: %d %d\n",m_sac_lower_bound,m_sac_upper_bound);
	fprintf(cfg_file,"Sac scale nom: %d\n",m_sac_scale_nominator);
	fprintf(cfg_file,"Sac safe velocity: %d\n",m_sac_safe_velocity);
	fprintf(cfg_file,"Sac safe duration: %d\n",m_sac_safe_duration);
	fprintf(cfg_file,"Fix safe deg: %Lf\n",m_fix_safe_deg);	
	fprintf(cfg_file,"Region tol deg: %Lf %Lf\n\n",m_region_tol_deg_x, m_region_tol_deg_y);
	fprintf(cfg_file,"Time-out: %d\n\n",m_timeout_limit);

	fprintf(cfg_file,"Cal points n: %d\n",m_cal_points_n);
	fprintf(cfg_file,"Cal samples: %d %d\n",m_cal_samples_pre_n, m_cal_samples_post_n);
	fprintf(cfg_file,"Cal range deg: %Lf %Lf\n",m_cal_range_deg_x,m_cal_range_deg_y);
	fprintf(cfg_file,"Cal center offset deg: %Lf %Lf\n",m_cal_center_offset_deg_x,m_cal_center_offset_deg_y);
	fprintf(cfg_file,"Cal drift deg: %Lf %Lf\n",m_cal_drift_deg_x,m_cal_drift_deg_y);
	fprintf(cfg_file,"Cal flocktol deg: %Lf %Lf\n\n",m_cal_flocktol_deg_x,m_cal_flocktol_deg_y);
	
	fprintf(cfg_file,"Cal points daqx daqy pixx pixy:\n");
	for (unsigned int i=0; i <m_cal_daqvals_x.size(); i++)
	{
		fprintf(cfg_file,"%d %d %d %d\n",m_cal_daqvals_x[i], m_cal_daqvals_y[i], m_cal_pixvals_x[i], m_cal_pixvals_y[i]);
	}
	
	fprintf(cfg_file,"\nCal fit:");
	for (it=m_cal_fit.begin(); it < m_cal_fit.end(); it++)
	{
		fprintf(cfg_file," %Lf",*it);
	}

	fprintf(cfg_file,"\nCal params:");
	for (it=m_cal_params.begin(); it < m_cal_params.end(); it++)
	{
		fprintf(cfg_file," %Lf",*it);
	}

	fprintf(cfg_file,"Cal drift: %d %d\n",m_cal_totdrift_pix_x,m_cal_totdrift_pix_y);

	fclose(cfg_file);	
	return true;
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_config::load()
//	DESCRIPTION:	Load config from a file
//--------------------------------------------------------------------
bool ecl_config::load(string fname)
{
	FILE * cfg_file;
	char buf[100];
	
	if ((cfg_file = fopen(fname.c_str(), "r")) == NULL) 
	{
		ecl_error("ECL: ecl_config_stddpi::load() - Cannot open config file");
		return false;
	}
	
	char tmpcharr[20];
	fgets(buf,100,cfg_file);
	if(!sscanf(buf,"Config ID: %s", tmpcharr))
	{
		ecl_error("ECL: ecl_config_stddpi::load() - Cannot parse line");
		return false;
	}
	m_cfg_id = tmpcharr;
	
/*	double ver = 0.0;
	if (!sscanf(m_cfg_id,"STDDPI-%Lf",ver) )
	{
		ecl_error("ECL: ecl_config_stddpi::load() - Incompatible config type");
		return false;
	}

	sprintf(m_cfg_id,"%s","STDDPI-1.0");*/

	fgets(buf,100,cfg_file);

	fgets(buf,100,cfg_file);
	if(!sscanf(buf,"Sac detect: %d",&m_sac_detect)){ecl_error("ECL: ecl_config_stddpi::load() - Cannot parse line");return false;}
	fgets(buf,100,cfg_file);
	if(sscanf(buf,"Sac boundaries: %d %d",&m_sac_lower_bound,&m_sac_upper_bound)<2){ecl_error("ECL: ecl_config_stddpi::load() - Cannot parse line");return false;}
	fgets(buf,100,cfg_file);
	if(sscanf(buf,"Sac scale nom: %d",&m_sac_scale_nominator)<1){ecl_error("ECL: ecl_config_stddpi::load() - Cannot parse line");return false;}
	fgets(buf,100,cfg_file);
	if(!sscanf(buf,"Sac safe velocity: %d",&m_sac_safe_velocity)){ecl_error("ECL: ecl_config_stddpi::load() - Cannot parse line");return false;}
	fgets(buf,100,cfg_file);
	if(!sscanf(buf,"Sac safe duration: %d",&m_sac_safe_duration)){ecl_error("ECL: ecl_config_stddpi::load() - Cannot parse line");return false;}
	fgets(buf,100,cfg_file);
	if(!sscanf(buf,"Fix safe deg: %Lf",&m_fix_safe_deg)){ecl_error("ECL: ecl_config_stddpi::load() - Cannot parse line");return false;}
	fgets(buf,100,cfg_file);
	if(sscanf(buf,"Region tol deg: %Lf %Lf",&m_region_tol_deg_x, &m_region_tol_deg_y)<2){ecl_error("ECL: ecl_config_stddpi::load() - Cannot parse line");return false;}
	fgets(buf,100,cfg_file);
	fgets(buf,100,cfg_file);
	if(!sscanf(buf,"Time-out: %d",&m_timeout_limit)){ecl_error("ECL: ecl_config_stddpi::load() - Cannot parse line");return false;}
	
	fgets(buf,100,cfg_file);
	
	fgets(buf,100,cfg_file);
	if(!sscanf(buf,"Cal points n: %d",&m_cal_points_n)){ecl_error("ECL: ecl_config_stddpi::load() - Cannot parse line");return false;}
	fgets(buf,100,cfg_file);
	if(sscanf(buf,"Cal samples: %d %d",&m_cal_samples_pre_n, &m_cal_samples_post_n)<2){ecl_error("ECL: ecl_config_stddpi::load() - Cannot parse line");return false;}
	fgets(buf,100,cfg_file);
	if(sscanf(buf,"Cal range deg: %Lf %Lf",&m_cal_range_deg_x,&m_cal_range_deg_y)<2){ecl_error("ECL: ecl_config_stddpi::load() - Cannot parse line");return false;}
	fgets(buf,100,cfg_file);
	if(sscanf(buf,"Cal center offset deg: %Lf %Lf",&m_cal_center_offset_deg_x,&m_cal_center_offset_deg_y)<2){ecl_error("ECL: ecl_config_stddpi::load() - Cannot parse line");return false;}
	fgets(buf,100,cfg_file);
	if(sscanf(buf,"Cal drift deg: %Lf %Lf",&m_cal_drift_deg_x,&m_cal_drift_deg_y)<2){ecl_error("ECL: ecl_config_stddpi::load() - Cannot parse line");return false;}
	fgets(buf,100,cfg_file);
	if(sscanf(buf,"Cal flocktol deg: %Lf %Lf",&m_cal_flocktol_deg_x,&m_cal_flocktol_deg_y)<2){ecl_error("ECL: ecl_config_stddpi::load() - Cannot parse line");return false;}
	
	fgets(buf,100,cfg_file);

	/*delete [] m_cal_daqvals_x;
	delete [] m_cal_daqvals_y;
	delete [] m_cal_pixvals_x;
	delete [] m_cal_pixvals_y;

	m_cal_daqvals_x = new (nothrow) long[m_cal_points_n];
	m_cal_daqvals_y = new (nothrow) long[m_cal_points_n];
	m_cal_pixvals_x = new (nothrow) long[m_cal_points_n];
	m_cal_pixvals_y = new (nothrow) long[m_cal_points_n];
		
	if (m_cal_daqvals_x==NULL || m_cal_daqvals_y==NULL || m_cal_pixvals_x == NULL || m_cal_pixvals_y == NULL)
	{
		ecl_error("Cannot allocate memory!");
	}*/
	
	m_cal_daqvals_x.clear();
	m_cal_daqvals_y.clear();
	m_cal_pixvals_x.clear();
	m_cal_pixvals_y.clear();

	m_cal_daqvals_x.reserve(m_cal_points_n);
	m_cal_daqvals_y.reserve(m_cal_points_n);
	m_cal_pixvals_x.reserve(m_cal_points_n);
	m_cal_pixvals_y.reserve(m_cal_points_n);

	fgets(buf,100,cfg_file);
	sscanf(buf,"Cal points daqx daqy pixx pixy:");
	long tmpa, tmpb, tmpc, tmpd;
	for (unsigned int i=0; i <m_cal_points_n; i++)
	{
		fgets(buf,100,cfg_file);
		if(sscanf(buf,"%d %d %d %d",&tmpa,&tmpb,&tmpc,&tmpd)<4){ecl_error("ECL: ecl_config_stddpi::load() - Cannot parse line");return false;}
		
		m_cal_daqvals_x.push_back(tmpa);
		m_cal_daqvals_y.push_back(tmpb);
		m_cal_pixvals_x.push_back(tmpc);
		m_cal_pixvals_y.push_back(tmpd);
	}

	fgets(buf,100,cfg_file);
	
	/*fgets(buf,100,cfg_file);
	if(sscanf(buf,"Cal daqpix corr: %Lf %Lf",&m_cal_daqpix_corr_x,&m_cal_daqpix_corr_y)<2){ecl_error("ECL: ecl_config_stddpi::load() - Cannot parse line");return false;}
	fgets(buf,100,cfg_file);
	if(sscanf(buf,"Cal intercepts: %Lf %Lf",&m_cal_interc_x,&m_cal_interc_y)<2){ecl_error("ECL: ecl_config_stddpi::load() - Cannot parse line");return false;}
	fgets(buf,100,cfg_file);
	if(sscanf(buf,"Cal weights: %Lf %Lf",&m_cal_weight_x,&m_cal_weight_y)<2){ecl_error("ECL: ecl_config_stddpi::load() - Cannot parse line");return false;}*/
	
	string tmpstr, tmpstr2;

	m_cal_fit.clear();
	m_cal_params.clear();
	m_cal_fit.reserve(ECL_CAL_FIT_N);
	m_cal_params.reserve(ECL_CAL_PARAMS_N);

	fgets(buf,100,cfg_file);
	tmpstr = buf;
	tmpa = tmpstr.find(": ");

	if (tmpa!=7){ecl_error("ECL: ecl_config_stddpi::load() - Cannot parse line");return false;}
	
	tmpstr2 = tmpstr.substr(tmpa+2);
	do
	{		
		tmpa = tmpstr2.find(" ");
		
		if (tmpa != -1)
		{
			tmpstr = tmpstr2.substr(0,tmpa);
			m_cal_fit.push_back(atof(tmpstr.c_str()));
			tmpstr2 = tmpstr2.substr(tmpa+1);
		}
		else
		{
			m_cal_fit.push_back(atof(tmpstr2.c_str()));
		}
	}
	while (tmpa != -1);

	if (m_cal_fit.size() != ECL_CAL_FIT_N)
	{
		ecl_error("ECL: ecl_config_stddpi::load() - Too few calibration fit variables?");		
	}


	fgets(buf,100,cfg_file);
	tmpstr = buf;
	tmpa = tmpstr.find(": ");

	if (tmpa!=10){ecl_error("ECL: ecl_config_stddpi::load() - Cannot parse line");return false;}
	
	tmpstr2 = tmpstr.substr(tmpa+2);
	do
	{		
		tmpa = tmpstr2.find(" ");
		
		if (tmpa != -1)
		{
			tmpstr = tmpstr2.substr(0,tmpa);
			m_cal_params.push_back(atof(tmpstr.c_str()));
			tmpstr2 = tmpstr2.substr(tmpa+1);
		}
		else
		{
			m_cal_params.push_back(atof(tmpstr2.c_str()));
		}
	}
	while (tmpa != -1);

	if (m_cal_params.size() != ECL_CAL_PARAMS_N)
	{
		ecl_error("ECL: ecl_config_stddpi::load() - Too few calibration parameters?");		
	}
	

	fgets(buf,100,cfg_file);
	if(!sscanf(buf,"Cal drift: %d %d",&m_cal_totdrift_pix_x, &m_cal_totdrift_pix_y)){ecl_error("ECL: ecl_config_stddpi::load() - Cannot parse line");return false;}

	fclose(cfg_file);
	
	time(&m_cal_time);
	m_b_initialised = true;

	return true;
}
//