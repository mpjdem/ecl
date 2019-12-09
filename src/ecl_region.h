#ifndef _ECL_REGION_INCLUDED_
#define _ECL_REGION_INCLUDED_

#include "ecl_monitor.h"

using namespace std;



//--------------------------------------------------------------------
//  CLASS:			ecl_region
//	DESCRIPTION:	Screen region definition base class
//--------------------------------------------------------------------
class ecl_region
{
public:

															ecl_region();
	virtual												  ~ ecl_region();
	

	// Base class implemented

			bool											set();	// Does nothing but return false
			bool											is_set(){return m_b_set;}
			inline	bool									set_tolerance(long tol_pix){m_tolerance_pix=tol_pix;}
			inline	long									get_center_x(){return m_center_x;}
			inline	long									get_center_y(){return m_center_y;}

			// Preloading functions
			bool											preload_image(ecl_monitor * mon, ecl_LUTc c_on, ecl_LUTc c_off, double scale);
			bool											use_as_preload_image(ecl_monitor * mon, long handle_on, long handle_off);		
			long											fetch_handle(bool eye_on);
			bool											draw(bool eye_on,long center_x=-999, long center_y=-999);
			bool											draw(long x, long y,long center_x=-999, long center_y=-999);
			bool											delete_preloaded_image();
			
			// On-the-fly functions
			bool											draw(ecl_monitor * mon, ecl_LUTc c, long center_x=-999, long center_y=-999);


	// Child class implemented
	virtual	bool											in_region(long x, long y)=0;


protected:
	// Base class implemented
			bool											m_b_set;
			long											m_tolerance_pix;
			long											m_preload_handle_on;
			long											m_preload_handle_off;
			ecl_monitor									*	m_preload_mon;

			long											m_rect_width;
			long											m_rect_height;
			long											m_center_x;
			long											m_center_y;
			

	// Child class implemented	
	virtual	bool											draw_func(unsigned long preload, long center_x, long center_y, ecl_monitor * mon=NULL, ecl_LUTc c=-1)=0;

};
//

#endif	//_ECL_REGION_INCLUDED_