#include "stdafx.h"

#include "ecl_region_circle.h"

using namespace std;


//--------------------------------------------------------------------
//	FUNCTION:		ecl_region_circle::ecl_region_circle()
//	DESCRIPTION:	Constructor
//--------------------------------------------------------------------
ecl_region_circle::ecl_region_circle()
{
	m_radius = 0;
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_region_circle::ecl_region_circle()
//	DESCRIPTION:	Constructor
//--------------------------------------------------------------------
ecl_region_circle::ecl_region_circle(long center_x, long center_y, long radius)
{
	set(center_x,  center_y, radius);
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_region_circle::~ecl_region_circle()
//	DESCRIPTION:	Deconstructor
//--------------------------------------------------------------------
ecl_region_circle::~ecl_region_circle()
{
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_region_circle::set()
//	DESCRIPTION:	Set the region parameters
//--------------------------------------------------------------------
bool ecl_region_circle::set(long center_x, long center_y, long radius)
{
	if (m_b_set && m_preload_mon)
	{
		delete_preloaded_image();
	}

	if (m_center_x < 0 || m_center_y < 0 || m_radius < 0)
	{
		ecl_error("ECL: ecl_region_circle::set() - All arguments must be positive");
		return false;
	}

	m_center_x = center_x;
	m_center_y = center_y;
	m_radius = radius;
	m_rect_width = (2*radius) + 2;
	m_rect_height = (2*radius) + 2;
	m_b_set = true;

	return true;
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_region_circle::in_region()
//	DESCRIPTION:	Determine whether this coordinate is inside the region
//--------------------------------------------------------------------
bool ecl_region_circle::in_region(long x, long y)
{
	if(!m_b_set)
	{
		ecl_error("ECL: ecl_region_circle::in_region() - Region parameters have not been set");
		return false;
	}

	double dist_from_center = sqrt((double)(((m_center_x-x)*(m_center_x-x))+((m_center_y-y)*(m_center_y-y))));

	if (dist_from_center > m_radius + m_tolerance_pix)
		{return false;}
	else
		{return true;}
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_region_circle::draw_func()
//	DESCRIPTION:	Draw a filled circle, or a preloaded image
//--------------------------------------------------------------------
bool ecl_region_circle::draw_func(unsigned long preload, long center_x, long center_y, ecl_monitor * mon, ecl_LUTc c)
{
	// Most error checking is already done by the base class functions calling draw_func
	// So let's keep this one short and simple
	
	bool res;

	if (!preload)
	{
		res = mon->draw_oval(0, center_x, center_y, m_radius, m_radius, c);
	}
	else
	{
		res = m_preload_mon->blit_preloaded_image(preload, center_x, center_y);
	}

	return res;
}
//