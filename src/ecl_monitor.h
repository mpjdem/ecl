#ifndef _ECL_MONITOR_INCLUDED_
#define _ECL_MONITOR_INCLUDED_

#include <vector>
#include <string>

#include <windows.h>
#include "ecl.h"

using namespace std;


//--------------------------------------------------------------------
//  CLASS:			ecl_monitor
//	DESCRIPTION:	Monitor base class
//--------------------------------------------------------------------
class ecl_monitor
{
public:

															ecl_monitor();
	virtual												  ~ ecl_monitor();

	// Base class implemented
	inline	bool											is_initialised(){return m_b_initialised;}
	inline	string											get_name(){return m_mon_name;}

	inline	long											get_width_pix(){return m_width_pix;}
	inline	long											get_height_pix(){return m_height_pix;}
	inline	long											get_width_mm(){return m_width_mm;}
	inline	long											get_height_mm(){return m_height_mm;}
	inline	long											get_distance_mm(){return m_distance_mm;}
	inline	long											get_distance_pix(){return m_distance_pix;}

	inline	long											get_framerate_hz(){return m_framerate_hz;}
	inline	long											get_color_depth(){return m_color_depth;}

	inline	long											get_font_size(){return m_font_size;}
	inline	string											get_font_name(){return m_font_name;}
	inline	ecl_LUTc										get_bg_color(){return m_bg_color;}
	inline	ecl_LUTc										get_fg_color(){return m_fg_color;}
	inline	long											get_throttle(){return m_throttle_hz;}

	inline	void											set_bg_color(ecl_LUTc c){m_bg_color = c;}
	inline	void											set_fg_color(ecl_LUTc c){m_fg_color = c;}
	inline	void											set_font(string font_name, long font_size){m_font_name = font_name; m_font_size = font_size; create_font();}

	inline	void											set_throttle(long hz){m_throttle_hz = hz;}

			double											compute_visual_angle(double size, double distance);

			void											wait_for_present();


    // Child class implemented
	virtual	bool											init(HINSTANCE instance, string name, long mon_n, long width_pix, long height_pix, long hz, long cdepth, long width_mm, long height_mm, long distance_mm)=0;
	virtual	void											cleanup()=0;
	virtual	bool											present()=0;

	virtual	bool											clear_screen()=0;
	//virtual	bool											clear_rect(long preload, long center_x, long center_y, long w, long h)=0;
			
	virtual	bool											draw_rect(unsigned long preload, long center_x, long center_y, long w, long h, ecl_LUTc c=-1, long frame_width=-1, ecl_LUTc inframe_c=-1)=0;
	virtual	bool											draw_oval(unsigned long preload, long center_x, long center_y, long w_rad, long h_rad, ecl_LUTc fill_c=-1, long frame_width=-1, ecl_LUTc inframe_c=-1)=0;
	virtual bool											draw_text(unsigned long preload, string text, long x0, long y0, ecl_LUTc c_fg=-1, ecl_LUTc c_bg=-1)=0;


	virtual	unsigned long									create_preloaded_image(long w, long h)=0;
	virtual bool											is_valid_preloaded_image(unsigned long handle)=0;
	virtual	bool											blit_preloaded_image(unsigned long handle, long center_x, long center_y)=0;
	virtual	bool											delete_preloaded_image(unsigned long handle)=0;
	virtual	bool											delete_all_preloaded_images()=0;


protected:

	// Base class implemented
			void											clip(long & src_x0, long & src_y0, long & src_w, long & src_h, long dst_x0, long dst_y0, long dst_w, long dst_h);

			bool											m_b_initialised;
			string											m_mon_name;

			long											m_width_pix;
			long											m_height_pix;
			long											m_width_mm;
			long											m_height_mm;
			long											m_distance_mm;
			long											m_distance_pix;
			long											m_framerate_hz;
			long											m_color_depth;

			long											m_throttle_hz;

			ecl_LUTc										m_bg_color;
			ecl_LUTc										m_fg_color;
			string											m_font_name;
			long											m_font_size;

			long											m_present_t;

    // Child class implemented
	virtual	bool											create_font()=0;
};

#endif	//_ECL_MONITOR_INCLUDED_
