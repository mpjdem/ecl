#include "stdafx.h"

#include <math.h>

#include "ecl.h"
#include "ecl_monitor.h"

using namespace std;


//--------------------------------------------------------------------
//	FUNCTION:		ecl_monitor::ecl_monitor()
//	DESCRIPTION:	Constructor
//--------------------------------------------------------------------
ecl_monitor::ecl_monitor()
{
	m_b_initialised = false;
	m_mon_name = "UNINITIALISED";

	m_width_pix = 0;
	m_height_pix = 0;
	m_width_mm = 0;
	m_height_mm = 0;
	m_distance_mm = 0;
	m_distance_pix = 0;
	m_framerate_hz = 0;
	m_color_depth = 0;

	m_throttle_hz = 0;
	m_present_t = 0;

	m_bg_color = 0;
	m_fg_color = 0;
	m_font_name = "NONE";
	m_font_size = 0;
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_monitor::~ecl_monitor()
//	DESCRIPTION:	Deconstructor
//--------------------------------------------------------------------
ecl_monitor::~ecl_monitor()
{
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_monitor::compute_visual_angle()
//	DESCRIPTION:	Convert distance and size to visual angle
//--------------------------------------------------------------------
double ecl_monitor::compute_visual_angle(double size, double distance)
{
	double ang_rad;
    double ang_deg;

	if (size <= 0.0 || distance <= 0.0)
	{
		ecl_error("ECL: ecl_monitor::compute_visual_angle() - Size and distance must be positive");
		return -1;
	}

    ang_rad = atan(((double)size)/((double)distance));
    ang_deg = (int)floor(((180*ang_rad)/PI)+0.5);
    return ang_deg;
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_monitor::clip()
//	DESCRIPTION:	Clip rectangle copying
//--------------------------------------------------------------------
void ecl_monitor::clip(long & src_x0, long & src_y0, long & src_w, long & src_h, long dst_x0, long dst_y0, long dst_w, long dst_h)
{
	bool clipped = false;
	if (src_x0 < dst_x0)
		{src_x0=dst_x0; clipped=true;}
	if (src_y0 < dst_y0) 
		{src_y0=dst_y0; clipped=true;}
	if (src_x0 > dst_x0+dst_w)
		{src_x0 = dst_x0+dst_w-1; src_w=1; clipped = true;}
	if (src_y0 > dst_y0+dst_h)
		{src_y0 = dst_y0+dst_h-1; src_h=1; clipped = true;}
	if (src_x0 + src_w >= dst_x0+dst_w) 
		{src_w= src_w + ((dst_x0+dst_w)-(src_x0+src_w)-1); clipped = true;}
	if (src_y0 + src_h >= dst_y0+dst_h) 
		{src_h= src_h + ((dst_y0+dst_h)-(src_y0+src_h)-1); clipped = true;}
	
	if(clipped)
	{ecl_error("ECL: ecl_monitor::clip() - Clipped!");}

}
//



//--------------------------------------------------------------------
//	FUNCTION:		ecl_monitor::wait_for_present()
//	DESCRIPTION:	Wait until a present is possible
//--------------------------------------------------------------------
void ecl_monitor::wait_for_present()
{
	long t = ecl_get_time();
	long min_dt = ECL_SAMPLE_FREQ/m_throttle_hz;

	while(abs(t-m_present_t) < min_dt)
	{}
}
//
