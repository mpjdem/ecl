#ifndef _ECL_MONITOR_DX_INCLUDED_
#define _ECL_MONITOR_DX_INCLUDED_

#include "dx\ddutil.h"

#include "ecl.h"
#include "ecl_monitor.h"
#include "ecl_dxmanager.h"

using namespace std;


//--------------------------------------------------------------------
//  CLASS:			ecl_monitor_dx
//	DESCRIPTION:	DirectX monitor child class
//--------------------------------------------------------------------
class ecl_monitor_dx : public ecl_monitor, public CDisplay
{
public:

															ecl_monitor_dx();
	virtual												  ~ ecl_monitor_dx();

	// Base class implementations
			bool											init(HINSTANCE instance, string name, long mon_n, long width_pix, long height_pix, long hz, long cdepth, long width_mm, long height_mm, long distance_mm);
			void											cleanup();
			bool											present();

			bool											clear_screen();
			bool											clear_rect(long center_x, long center_y, long w, long h);
			
			bool											draw_rect(unsigned long preload, long center_x, long center_y, long w, long h, ecl_LUTc c=-1, long frame_width=-1, ecl_LUTc inframe_c=-1);
			bool											draw_oval(unsigned long preload, long center_x, long center_y, long w_rad, long h_rad, ecl_LUTc fill_c=-1, long frame_width=-1, ecl_LUTc inframe_c=-1);
			bool											draw_text(unsigned long preload, string text, long x0, long y0, ecl_LUTc c_fg=-1, ecl_LUTc c_bg=-1);


/*			bool											wait_for_vertical_blank();
			long											draw_line(bool preload, long x1, long y1, long x2, long y2, ecl_LUTc c=-1);
			*/

/*
	virtual	long											draw_arc(bool preload, long center_x, long center_y, long outer_r, long inner_r, long begin_angle, long end_angle, ecl_LUTc fill_c=-1)=0;
	virtual	long											draw_cross(bool preload, long center_x, long center_y, long w, long h, long b, long or, ecl_LUTc fill_c=-1)=0;

	virtual	long											draw_bitmap(bool preload, HBITMAP bmp,long center_x, long center_y, long w, long h)=0;
	virtual bool											draw_pixel(bool preload, long x, long y, ecl_LUTc c=-1)=0;
*/

			unsigned long									create_preloaded_image(long w, long h);
			bool											blit_preloaded_image(unsigned long handle, long center_x, long center_y);
			bool											is_valid_preloaded_image(unsigned long handle);
			bool											delete_preloaded_image(unsigned long handle);
			bool											delete_all_preloaded_images();


	// Own implementations
			void											free_ddraw();

protected:

	// Base class implementations
			bool											create_font();


	// Own implementations
		static	LRESULT CALLBACK							WindowProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

				vector<CSurface*>							m_preloaded_images;
				HFONT										m_font;

				ecl_dxmanager								m_g_dxman;
		static	HWND										m_g_dxwnd;


};

#endif	//_ECL_MONITOR_DX_INCLUDED_