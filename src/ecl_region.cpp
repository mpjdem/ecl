#include "stdafx.h"

#include "ecl_region.h"

using namespace std;


//--------------------------------------------------------------------
//	FUNCTION:		ecl_region::ecl_region()
//	DESCRIPTION:	Constructor
//--------------------------------------------------------------------
ecl_region::ecl_region()
{
	m_b_set = false;
	m_tolerance_pix = 0;
	m_preload_handle_on = -1;
	m_preload_handle_off = -1;
	m_preload_mon = NULL;
	m_rect_width = 0;
	m_rect_height = 0;

	m_center_x = 0;
	m_center_y = 0;
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_region::~ecl_region()
//	DESCRIPTION:	Deconstructor
//--------------------------------------------------------------------
ecl_region::~ecl_region()
{
	delete_preloaded_image();
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_region::set()
//	DESCRIPTION:	Set parameters. Unused in the base class.
//--------------------------------------------------------------------
bool ecl_region::set()
{
	ecl_error("ECL: ecl_region::set() - Arguments must be specified");
	return false;
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_region::preload_image()
//	DESCRIPTION:	Create preloaded image from parameters
//--------------------------------------------------------------------
bool ecl_region::preload_image(ecl_monitor * mon, ecl_LUTc c_on, ecl_LUTc c_off, double scale)
{
	bool res;

	if(!m_b_set)
	{
		ecl_error("ECL: ecl_region::preload_image() - Region parameters have not been set");
		return false;
	}

	if(mon == NULL)
	{	
		ecl_error("ECL: ecl_region::preload_image - Invalid monitor");
        return false;
	}

	if(m_preload_mon != NULL)
	{	
		ecl_error("ECL: ecl_region::preload_region - A preload image already exists");
        return false;
	}

	m_preload_mon = mon;

	m_preload_handle_on = mon->create_preloaded_image((long)((double)m_rect_width*scale), (long)((double)m_rect_height*scale));

	if (m_preload_handle_on == -1)
	{
		m_preload_mon = NULL;
		ecl_error("ECL: ecl_region::preload_image - Cannot preload image");
		return false;
	}

	m_preload_handle_off = mon->create_preloaded_image((long)((double)m_rect_width*scale), (long)((double)m_rect_height*scale));

	if (m_preload_handle_off == -1)
	{
		mon->delete_preloaded_image(m_preload_handle_on);
		m_preload_mon = NULL;
		ecl_error("ECL: ecl_region::preload_image - Cannot preload image");
		return false;
	}

	res = draw_func(m_preload_handle_on,(long)((double)m_rect_width*scale)/2,(long)((double)m_rect_height*scale)/2,m_preload_mon,c_on);

	if (!res)
	{
		m_preload_mon = NULL;
		ecl_error("ECL: ecl_region::preload_image - Cannot preload image");
		return false;
	}

	res = draw_func(m_preload_handle_off,(long)((double)m_rect_width*scale)/2,(long)((double)m_rect_height*scale)/2,m_preload_mon,c_off);

	if (!res)
	{
		m_preload_mon = NULL;
		ecl_error("ECL: ecl_region::preload_image - Cannot preload image");
		return false;
	}

	return true;	
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_region::use_as_preload_image()
//	DESCRIPTION:	Use an existing preloaded image to render the region
//--------------------------------------------------------------------
bool ecl_region::use_as_preload_image(ecl_monitor * mon, long handle_on, long handle_off)
{
	if(!m_b_set)
	{
		ecl_error("ECL: ecl_region::use_as_preload_image() - Region parameters have not been set");
		return false;
	}

	if(mon == NULL)
	{	
		ecl_error("ECL: ecl_region::use_as_preload_image - Invalid monitor");
        return false;
	}

	if(m_preload_mon != NULL)
	{	
		ecl_error("ECL: ecl_region::use_as_preload_image - A preload image already exists");
        return false;
	}

	m_preload_mon = mon;
	
	if (!mon->is_valid_preloaded_image(handle_on))
	{
		m_preload_mon = NULL;
		ecl_error("ECL: ecl_region::use_as_preload_image - Invalid handle");
		return false;
	}

	m_preload_handle_on = handle_on;

	if (!mon->is_valid_preloaded_image(handle_off))
	{
		m_preload_handle_on = -1;
		m_preload_mon = NULL;
		ecl_error("ECL: ecl_region::use_as_preload_image - Invalid handle");
		return false;
	}

	m_preload_handle_off = handle_off;

	// Note that this preloaded image may have any size
	return true;
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_region::fetch_handle()
//	DESCRIPTION:	Return a specific handle
//--------------------------------------------------------------------
long ecl_region::fetch_handle(bool eye_on)
{
	if (eye_on)
	{
		return m_preload_handle_on;
	}
	else
	{
		return m_preload_handle_off;
	}
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_region::draw()
//	DESCRIPTION:	Blit the preloaded region; manual in_region check
//--------------------------------------------------------------------
bool ecl_region::draw(bool eye_on, long center_x, long center_y)
{
	long handle;
	bool res;

	if(!m_b_set)
	{
		ecl_error("ECL: ecl_region::draw() - Region parameters have not been set");
		return false;
	}

	if(m_preload_mon == NULL)
	{	
		ecl_error("ECL: ecl_region::draw - Invalid monitor");
        return false;
	}

	if (eye_on)
		{handle=m_preload_handle_on;}
	else
		{handle=m_preload_handle_off;}


	if (!m_preload_mon->is_valid_preloaded_image(handle))
	{
		ecl_error("ECL: ecl_region::draw - Invalid handle");
		return false;
	}

	if (center_x == -999 && center_y == -999)
	{
		center_x = m_center_x;
		center_y = m_center_y;
	}

	res = m_preload_mon->blit_preloaded_image(handle, center_x, center_y);

	if (!res)
	{
		ecl_error("ECL: ecl_region::draw - Cannot blit preloaded image");
		return false;
	}

	return true;
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_region::draw()
//	DESCRIPTION:	Draw the preloaded region; automatic in_region check
//--------------------------------------------------------------------
bool ecl_region::draw(long x, long y, long center_x, long center_y)
{
	return draw(in_region(x,y), center_x, center_y);
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_region::destroy_preloaded_image()
//	DESCRIPTION:	Remove the preloaded images from memory
//--------------------------------------------------------------------
bool ecl_region::delete_preloaded_image()
{
	bool res1, res2;
	
	if(m_preload_mon == NULL)
	{	
		ecl_error("ECL: ecl_region::destroy_preloaded_image - No valid monitor");
        return false;
	}

	res1 = m_preload_mon->delete_preloaded_image(m_preload_handle_on);
	res2 = m_preload_mon->delete_preloaded_image(m_preload_handle_off);
	m_preload_mon = NULL;

	return (res1 && res2);
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_region::draw()
//	DESCRIPTION:	Draw the region on the fly in the specified color
//--------------------------------------------------------------------
bool ecl_region::draw(ecl_monitor * mon, ecl_LUTc c,  long center_x, long center_y)
{
	bool res;

	if(!m_b_set)
	{
		ecl_error("ECL: ecl_region::draw() - Region parameters have not been set");
		return false;
	}

	if(mon == NULL)
	{	
		ecl_error("ECL: ecl_region::draw() - Invalid monitor");
        return false;
	}

	res = draw_func(0, center_x, center_y, mon, c);

	if (!res)
	{
		ecl_error("ECL: ecl_region::draw() - Cannot draw region");
		return false;
	}

	return true;
}
//