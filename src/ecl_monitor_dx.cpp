#include "stdafx.h"

#include "dx\ddutil.h"

#include "ecl.h"
#include "ecl_monitor_dx.h"
#include "ecl_dxmanager.h"

using namespace std;


// Static variables
HWND ecl_monitor_dx::m_g_dxwnd;


//--------------------------------------------------------------------
//	FUNCTION:		ecl_monitor_dx::ecl_monitor_dx()
//	DESCRIPTION:	Constructor
//--------------------------------------------------------------------
ecl_monitor_dx::ecl_monitor_dx()
{
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_monitor_dx::~ecl_monitor_dx()
//	DESCRIPTION:	Deconstructor
//--------------------------------------------------------------------
ecl_monitor_dx::~ecl_monitor_dx()
{
	cleanup();
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_monitor_dx::clean_up()
//	DESCRIPTION:	Clean up this DirectX window
//--------------------------------------------------------------------
void ecl_monitor_dx::cleanup()
{
	free_ddraw();
	
	if ((m_g_dxwnd != NULL))
	{
		::CloseWindow(m_g_dxwnd);
		::DestroyWindow(m_g_dxwnd); 
	}
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_monitor_dx::init()
//	DESCRIPTION:	Initialization routine
//--------------------------------------------------------------------
bool ecl_monitor_dx::init(HINSTANCE instance, string name, long mon_n, long width_pix, long height_pix, long hz, long cdepth, long width_mm, long height_mm, long distance_mm)
{
	// Initialise through arguments, unchangeable later on
	m_mon_name = name;
	m_width_pix = width_pix;
	m_height_pix = height_pix;
	m_width_mm = width_mm;
	m_height_mm = height_mm;
	m_distance_mm = distance_mm;
	m_distance_pix = (distance_mm*height_pix)/height_mm;
	m_framerate_hz = hz;
	m_color_depth = cdepth;


    // Initialise to default arguments, can be changed after initialisation
	m_throttle_hz = hz;
	m_present_t = ecl_get_time();
	m_bg_color = ecl_make_LUTcolor(0,0,0);
	m_fg_color = ecl_make_LUTcolor(255,255,255);
	m_font_name = "TIMES NEW ROMAN";
	m_font_size = 10;


	// Find DirectX devices
	m_g_dxman.init();


	// Retrieve the right GUID pointer
	GUID * pGUID = m_g_dxman.get_monitor_guid(mon_n);
	if (pGUID == NULL)
	{
		ecl_error("ECL: ecl_monitor_dx::init() - Could not find monitor");
		return false;
	}


    // Remember that we're already using this device
	if (!m_g_dxman.allocate_monitor(mon_n))
	{
        return false;
	}


	// Create the DirectX window
    WNDCLASSEX wc;

	int bufferlen = ::MultiByteToWideChar(CP_ACP, 0, m_mon_name.c_str(), m_mon_name.size(), NULL, 0);
	LPWSTR mon_name_w = new WCHAR[bufferlen + 1];
	::MultiByteToWideChar(CP_ACP, 0,  m_mon_name.c_str(),  m_mon_name.size(), mon_name_w, bufferlen);
	mon_name_w[bufferlen] = 0;

	wc.cbSize        = sizeof(wc);
	wc.lpszClassName = mon_name_w;
	wc.lpfnWndProc   = WindowProc;
	wc.style         = CS_VREDRAW | CS_HREDRAW;
	wc.hInstance     = instance;
	wc.hIcon         = NULL;
	wc.hIconSm       = NULL;
	wc.hCursor       = LoadCursor( NULL, IDC_ARROW );
	wc.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
	wc.lpszMenuName  = NULL;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;

	if(!RegisterClassEx( &wc ))
	{
	    ecl_error("ECL: ecl_monitor_dx::init() - Could not register DirectX Window");
	    return false;
	}

	DWORD dwStyle = WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX;

	if (m_g_dxman.get_allocated_monitor_count() == 1)
	{
	    m_g_dxwnd = CreateWindowEx( 0, mon_name_w, mon_name_w, dwStyle, 0, 0, m_width_pix, m_height_pix, NULL, NULL, instance, NULL );

		if (m_g_dxwnd == NULL)
		{
			ecl_error("ECL: ecl_monitor_dx::init() - Could not create DirectX Window");
			return false;
		}

		::SetWindowLong(m_g_dxwnd, GWL_USERDATA, (long)this);
	    ::ShowWindow(m_g_dxwnd, SW_SHOW);
		::SetFocus(m_g_dxwnd);
	}
		delete[] mon_name_w;

	// Initialize DirectDraw
	HRESULT hr;
	DWORD dwFlags;
	DDSURFACEDESC2      ddsd;
    DDSCAPS2            ddsCaps;

	if( FAILED(hr = DirectDrawCreateEx( pGUID, (VOID**) &(m_pDD), IID_IDirectDraw7, NULL ) ) )
	{
		ecl_error("ECL: ecl_monitor_dx::init() - Could not initialize DirectDraw");
		return false;
	}

	if (m_g_dxman.get_allocated_monitor_count() == 1)
	{
		dwFlags = DDSCL_SETFOCUSWINDOW;
		if( FAILED( hr = m_pDD->SetCooperativeLevel( m_g_dxwnd, dwFlags ) ) )
		{
			ecl_error("ECL: ecl_monitor_dx::init() - Could not initialize DirectDraw");
			return false;
		}

		dwFlags = DDSCL_ALLOWREBOOT | DDSCL_FULLSCREEN | DDSCL_EXCLUSIVE;
		if( FAILED( hr = m_pDD->SetCooperativeLevel(m_g_dxwnd, dwFlags ) ) )
		{
			ecl_error("ECL: ecl_monitor_dx::init() - Could not initialize DirectDraw");
			return false;
		}
	}
	else
	{
		dwFlags = DDSCL_SETFOCUSWINDOW | DDSCL_CREATEDEVICEWINDOW | DDSCL_ALLOWREBOOT | DDSCL_FULLSCREEN | DDSCL_EXCLUSIVE ;
		if( FAILED( hr = m_pDD->SetCooperativeLevel(m_g_dxwnd, dwFlags ) ) )
        {
            ecl_error("ECL: ecl_monitor_dx::init() - Could not initialize DirectDraw");
            return false;
        }
	}

	if( FAILED( hr = m_pDD->SetDisplayMode(m_width_pix, m_height_pix, 32, 0, 0 ) ) )
	{
		return false;
	}

	ZeroMemory( &ddsd, sizeof(ddsd) );
    ddsd.dwSize            = sizeof(ddsd);
    ddsd.dwFlags           = DDSD_BACKBUFFERCOUNT | DDSD_CAPS;
    ddsd.dwBackBufferCount = 1;
    ddsd.ddsCaps.dwCaps    = DDSCAPS_COMPLEX | DDSCAPS_FLIP | DDSCAPS_PRIMARYSURFACE;
	if( FAILED( hr = m_pDD->CreateSurface( &ddsd, &(m_pddsFrontBuffer), NULL ) ) )
	{
		ecl_error("ECL: ecl_monitor_dx::init() - Could not create DirectDraw frontbuffer");
		return false;
	}

    ZeroMemory( &ddsCaps, sizeof(ddsCaps) );
    ddsCaps.dwCaps = DDSCAPS_BACKBUFFER;
    if( FAILED( hr = m_pddsFrontBuffer->GetAttachedSurface( &ddsCaps, &m_pddsBackBuffer ) ) )
	{
        ecl_error("ECL: ecl_monitor_dx::init() - Could not create DirectDraw backbuffer");
        return false;
	}

    ZeroMemory( &ddsd, sizeof(ddsd) );
    ddsd.dwSize = sizeof(ddsd);
    if( FAILED( hr = m_pddsFrontBuffer->GetSurfaceDesc( &ddsd ) ) )
	{
        ecl_error("ECL: ecl_monitor_dx::init() - Could not access DirectDraw frontbuffer");
        return false;
	}
	SetRect( &m_rcWindow, 0, 0, ddsd.dwWidth, ddsd.dwHeight );

    m_bWindowed = FALSE;


	// Clear the DirectDraw buffers to bg color
    static DDBLTFX ddbltfx;
	ZeroMemory( &ddbltfx, sizeof(ddbltfx) );
	ddbltfx.dwSize      = sizeof(ddbltfx);
	ddbltfx.dwFillColor = ConvertGDIColor(m_bg_color);
    if( FAILED( hr = m_pddsFrontBuffer->Blt( NULL, NULL, NULL, DDBLT_COLORFILL, &ddbltfx ) ) )
    {
        ecl_error("ECL: ecl_monitor_dx::init() - Could not blit DirectDraw frontbuffer");
        return false;
    }
    if( FAILED( hr = m_pddsBackBuffer->Blt( NULL, NULL, NULL, DDBLT_COLORFILL, &ddbltfx ) ) )
    {
        ecl_error("ECL: ecl_monitor_dx::init() - Could not blit DirectDraw backbuffer");
        return false;
    }
	

	// Re-read the settings from the monitor itself, so the experimenter may detect if anything is wrong
	RECT r = {0,0,0,0};
	m_g_dxman.get_monitor_rect(mon_n,&r);
	m_width_pix = r.right - r.left;
	m_height_pix = r.bottom - r.top;


	// Done
	m_b_initialised = true;
	return true;
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_monitor_dx::WindowProc()
//	DESCRIPTION:	Callback function for Windows messages
//--------------------------------------------------------------------
LRESULT CALLBACK ecl_monitor_dx::WindowProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	ecl_monitor_dx * dx = (ecl_monitor_dx*)GetWindowLong(hWnd, GWL_USERDATA);
    switch(uMsg)
    {
    case WM_SIZE:
        if( SIZE_MAXHIDE == wParam || SIZE_MINIMIZED == wParam )
        {
            // Give window an icon and system menu on the taskbar when minimized
            SetWindowLong(hWnd, GWL_STYLE, WS_SYSMENU); 
        }
        else
        {
            // Remove any window "decoration" when fullscreen
            SetWindowLong(hWnd, GWL_STYLE, WS_POPUP);
        }

        return DefWindowProc( hWnd, uMsg, wParam, lParam );

	case WM_DESTROY:
        dx -> free_ddraw();
        return 1;

	case WM_CLOSE:
		::CloseWindow(hWnd);
		::DestroyWindow(hWnd);
        return 0L;
		break;

    default:
        return DefWindowProc( hWnd, uMsg, wParam, lParam );
    }
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_monitor_dx::free_ddraw()
//	DESCRIPTION:	Free this monitor
//--------------------------------------------------------------------
void ecl_monitor_dx::free_ddraw()
{
	// Destroy all surfaces
	delete_all_preloaded_images();

	// Release the buffers
	if (m_pddsBackBuffer != NULL)
	{
		m_pddsBackBuffer->Release();
		m_pddsBackBuffer = NULL;
	}
	if (m_pddsFrontBuffer != NULL)
	{
		m_pddsFrontBuffer->Release();
		m_pddsFrontBuffer = NULL;
	}

	// Release the screen
	if (m_pDD != NULL)
	{
		m_pDD->RestoreDisplayMode();
		m_pDD->SetCooperativeLevel(m_g_dxwnd, DDSCL_NORMAL);
		m_pDD->Release();
		m_pDD = NULL;
	}
}


//--------------------------------------------------------------------
//	FUNCTION:		ecl_monitor_dx::present()
//	DESCRIPTION:	Flip the buffers
//--------------------------------------------------------------------
bool ecl_monitor_dx::present()
{
	if(!is_initialised())
	{
		// ERROR
		return false;
	}
	
	long t = ecl_get_time();
	long min_dt = ECL_SAMPLE_FREQ/m_throttle_hz;

	if (abs(t-m_present_t) < min_dt)
		{return true;}
	else
	{	
		m_present_t = t;
		return SUCCEEDED(CDisplay::Present());
	}
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_monitor_dx::clear_screen()
//	DESCRIPTION:	Clear to the background color
//--------------------------------------------------------------------
bool ecl_monitor_dx::clear_screen()
{
    if(!is_initialised())
	{
		// ERROR
		return false;
	}
	
	static DDBLTFX ddbltfx;
	ZeroMemory( &ddbltfx, sizeof(ddbltfx) );
	ddbltfx.dwSize      = sizeof(ddbltfx);
	ddbltfx.dwFillColor = ConvertGDIColor(m_bg_color);

    m_pddsBackBuffer->Blt( NULL, NULL, NULL, DDBLT_COLORFILL, &ddbltfx );

	return true;
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_monitor_dx::clear_rect()
//	DESCRIPTION:	Clear a rectangular area to the background color
//--------------------------------------------------------------------
bool ecl_monitor_dx::clear_rect(long center_x, long center_y, long w, long h)
{
	if(!is_initialised())
	{
		ecl_error("ECL: ecl_monitor_dx::clear_rect() - Monitor is not initialised");
		return false;
	}
	
	RECT r = {center_x-(w/2),center_y-(h/2),center_x+(w/2),center_y+(h/2)};
	ClearRect(&r, ConvertGDIColor(m_bg_color));

	return true;
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_monitor_dx::draw_rect()
//	DESCRIPTION:	Draw a rectangular area
//--------------------------------------------------------------------
bool ecl_monitor_dx::draw_rect(unsigned long preload, long center_x, long center_y, long w, long h, ecl_LUTc c, long frame_width, ecl_LUTc inframe_c)
{
	if(!is_initialised())
	{
		ecl_error("ECL: ecl_monitor_dx::draw_rect() - Monitor is not initialised");
		return false;
	}
	
	long rx=center_x-(w/2),ry=center_y-(h/2),rh=h,rw=w;
	long irx=rx+frame_width,iry=ry+frame_width,irh=h-(2*frame_width),irw=w-(2*frame_width);
	bool framed = false;
	CSurface surf;
	int surf_w, surf_h;
	bool res = true;

 	if (c == -1)
		{c = m_fg_color;}

	if (inframe_c == -1)
		{inframe_c = m_bg_color;}

	if (frame_width > 0)
		{framed = true;}

 	if (preload)
	{	
		if (!is_valid_preloaded_image(preload))
		{
			ecl_error("ECL: ecl_region::draw_rect() - Invalid handle");
			return false;
		}

		surf = *m_preloaded_images[preload-1];
	}
	else
	{
		res = res && FAILED(surf.Attach(m_pddsBackBuffer));
	}

	surf_w = surf.GetWidth();
	surf_h = surf.GetHeight();
	clip(rx,ry,rw,rh,0,0,surf_w,surf_h);
	res = res && FAILED(surf.Rectangle(rx, ry, rw, rh, c));

	if (framed)
	{
		clip(irx,iry,irw,irh,0,0,surf_w,surf_h);
		res = res && FAILED(surf.Rectangle(rx, ry, rw, rh, inframe_c));
	}
		
	if (!preload)
	{
		res = res && FAILED(surf.Detach());
	}

	if (!res)
	{
		ecl_error("ECL: ecl_monitor_dx::draw_rect() - Could not perform drawing");
		return false;
	}

	return true;
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_monitor_dx::draw_oval()
//	DESCRIPTION:	Draw an oval area
//--------------------------------------------------------------------
bool ecl_monitor_dx::draw_oval(unsigned long preload, long center_x, long center_y, long w_rad, long h_rad, ecl_LUTc fill_c, long frame_width, ecl_LUTc inframe_c)
{
	if(!is_initialised())
	{
		ecl_error("ECL: ecl_monitor_dx::draw_oval() - Monitor is not initialised");
		return false;
	}
	
	long rx=center_x-(w_rad/2),ry=center_y-(h_rad/2),rh=h_rad*2,rw=w_rad*2;
	long irx=rx+frame_width,iry=ry+frame_width,irh=rh-(2*frame_width),irw=rw-(2*frame_width);
	bool framed = false;

	CSurface surf;
	long surf_w, surf_h;
	bool res = true;

 	if (fill_c == -1)
		{fill_c = m_fg_color;}

	if (inframe_c == -1)
		{inframe_c = m_bg_color;}

	if (frame_width > 0)
		{framed = true;}

 	if (preload)
	{	
		if (!is_valid_preloaded_image(preload))
		{
			ecl_error("ECL: ecl_monitor_dx::draw_oval() - Invalid handle");
			return false;
		}

		surf = *m_preloaded_images[preload-1];
	}
	else
	{
		res = res && FAILED(surf.Attach(m_pddsBackBuffer));
	}

	surf_w = surf.GetWidth();
	surf_h = surf.GetHeight();
	clip(rx,ry,rw,rh,0,0,surf_w,surf_h);
	res = res && FAILED(surf.Ellipse(rx,ry,rw,rh,fill_c,true));
	if (framed)
	{
		clip(irx,iry,irw,irh,0,0,surf_w,surf_h);
		res = res && FAILED(surf.Ellipse(rx,ry,rw,rh,inframe_c,true));
	}
		
	if (!preload)
	{
		res = res && FAILED(surf.Detach());
	}

	if (!res)
	{
		ecl_error("ECL: ecl_monitor_dx::draw_oval() - Could not perform drawing");
		return false;
	}

	return true;
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_monitor_dx::create_font()
//	DESCRIPTION:	Create a font out of the name and size
//--------------------------------------------------------------------
bool ecl_monitor_dx::create_font()
{
	if (m_font != NULL)
		::DeleteObject(m_font);
	
	LOGFONT	 logfont;
	logfont.lfHeight = m_font_size;
	logfont.lfWidth = 0;
	logfont.lfEscapement = 0;
	logfont.lfOrientation = 0;
	logfont.lfWeight = FW_NORMAL; 
	logfont.lfItalic = false;
	logfont.lfUnderline = false;
	logfont.lfStrikeOut = 0;
	logfont.lfCharSet = ANSI_CHARSET;
	logfont.lfOutPrecision = OUT_CHARACTER_PRECIS;
	logfont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	logfont.lfQuality = ANTIALIASED_QUALITY;//DEFAULT_QUALITY;
	logfont.lfPitchAndFamily = DEFAULT_PITCH;
	
	int bufferlen = ::MultiByteToWideChar(CP_ACP, 0, m_font_name.c_str(), m_font_name.size(), NULL, 0);
	LPWSTR font_name_w = new WCHAR[bufferlen + 1];
	::MultiByteToWideChar(CP_ACP, 0,  m_font_name.c_str(),  m_font_name.size(), font_name_w, bufferlen);
	font_name_w[bufferlen] = 0;
	wcscpy_s(logfont.lfFaceName, bufferlen, font_name_w);	
	
	m_font = ::CreateFontIndirect(&logfont);

	return true;
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_monitor_dx::draw_text()
//	DESCRIPTION:	Draw a string of text (slow!)
//--------------------------------------------------------------------
bool ecl_monitor_dx::draw_text(unsigned long preload, string text, long x0, long y0, ecl_LUTc c_fg, ecl_LUTc c_bg)
{
	if(!is_initialised())
	{
		ecl_error("ECL: ecl_monitor_dx::draw_text() - Monitor is not initialised");
		return false;
	}

	CSurface surf;
	// long surf_w, surf_h;
	bool res = true;

 	if (c_fg == -1)
		{c_fg = m_fg_color;}

	 if (c_bg == -1)
		{c_bg = m_bg_color;}

 	if (preload)
	{	
		if (!is_valid_preloaded_image(preload))
		{
			ecl_error("ECL: ecl_monitor_dx::draw_text() - Invalid handle");
			return false;
		}

		surf = *m_preloaded_images[preload-1];
	}
	else
	{
		res = res && FAILED(surf.Attach(m_pddsBackBuffer));
	}

	/*surf_w = surf->GetWidth();
	surf_h = surf->GetHeight();
	long xw = surf_w;
	long yh = y0 + m_font_size;

	clip(&x0,&y0,&xw,&yh,0,0,surf_w,surf_h);*/

	int bufferlen = ::MultiByteToWideChar(CP_ACP, 0, text.c_str(), text.size(), NULL, 0);
	LPWSTR t = new WCHAR[bufferlen + 1];
	::MultiByteToWideChar(CP_ACP, 0,  text.c_str(),  text.size(), t, bufferlen);
	t[bufferlen] = 0;

	res = res && FAILED(surf.DrawText(m_font,t, x0, y0, c_bg, c_fg));
		
	if (!preload)
	{
		res = res && FAILED(surf.Detach());
	}

	if (!res)
	{
		ecl_error("ECL: ecl_monitor_dx::draw_text() - Could not perform drawing");
		return false;
	}

	return true;
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_monitor_dx::create_preloaded_image()
//	DESCRIPTION:	Create a rectangular DirectDraw surface
//--------------------------------------------------------------------
unsigned long ecl_monitor_dx::create_preloaded_image(long w, long h)
{
	if(!is_initialised())
	{
		// ERROR
		return false;
	}
	
	CSurface *surf;
	HRESULT res = CreateSurface(&surf, w, h);

	if (FAILED(res))
	{
		delete surf;
		return -1;
	}
	else 
	{
		m_preloaded_images.push_back(surf);
		return m_preloaded_images.size();
	}
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_monitor_dx::blit_preloaded_image()
//	DESCRIPTION:	Copy preloaded image to the backbuffer
//--------------------------------------------------------------------
bool ecl_monitor_dx::blit_preloaded_image(unsigned long handle, long center_x, long center_y)
{
	if(!is_initialised())
	{
		// ERROR
		return false;
	}
		
	long px, py, pw, ph;
	long bbw, bbh;
	HRESULT res;
	CSurface * psurf;

	if (!is_valid_preloaded_image(handle))
	{
		ecl_error("ECL: ecl_region::blit_preloaded_image() - Invalid handle");
		return false;
	}

	psurf = m_preloaded_images[handle-1];
	pw = psurf->GetWidth();
	ph = psurf->GetHeight();
	px=center_x-(pw/2);
	py=center_y-(ph/2);

	bbw = get_width_pix();
	bbh = get_height_pix();

	if (px < 0 || py < 0 || px > bbw || py > bbh || px+pw >= bbw || py+ph >= bbh)
	{
		// ERROR
		return false;
	}

	// Blit onto backbuffer
	res = Blt(px,py, psurf);
		
	if (FAILED(res))
	{
		// ERROR
		return false;
	}

	return true;
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_monitor_dx::is_valid_preloaded_image()
//	DESCRIPTION:	Check whether this handle is valid
//--------------------------------------------------------------------
bool ecl_monitor_dx::is_valid_preloaded_image(unsigned long handle)
{
	if (handle > m_preloaded_images.size())
		{return false;}

	if (m_preloaded_images[handle-1] == NULL)
		{return false;}

	return true;
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_monitor_dx::delete_preloaded_image()
//	DESCRIPTION:	Remove this surface, leave a null pointer
//--------------------------------------------------------------------
bool ecl_monitor_dx::delete_preloaded_image(unsigned long handle)
{
	HRESULT res;
	
	if (!is_valid_preloaded_image(handle))
	{
		ecl_error("ECL: ecl_region::delete_preloaded_image() - Invalid handle");
		return false;
	}

	res = m_preloaded_images[handle-1]->Destroy();

	if (FAILED(res))
	{
		ecl_error("ECL: ecl_region::delete_preloaded_image() - Could not delete preloaded image");
		return false;
	}

	m_preloaded_images[handle-1] = NULL;

	return true;
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_monitor_dx::delete_all_preloaded_images()
//	DESCRIPTION:	Remove all surfaces, clear the vector
//--------------------------------------------------------------------
bool ecl_monitor_dx::delete_all_preloaded_images()
{
	bool res = true;

	for (unsigned int i = 0; i < m_preloaded_images.size(); i++)
	{
		if (m_preloaded_images[i] != NULL)
		{
			res = res && delete_preloaded_image(i+1);
		}
	}

	if(res)
	{
		m_preloaded_images.clear();
	}

	return res;
}
//