//-----------------------------------------------------------------------------
// File: ddutil.cpp
//
// Desc: Routines for loading bitmap and palettes from resources
//
// Copyright (C) 1998-1999 Microsoft Corporation. All Rights Reserved.
//-----------------------------------------------------------------------------
#ifndef DDUTIL_H
#define DDUTIL_H

#include <ddraw.h>
#include <d3d9.h>




//-----------------------------------------------------------------------------
// Classes defined in this header file 
//-----------------------------------------------------------------------------
class CDisplay;
class CSurface;




//-----------------------------------------------------------------------------
// Flags for the CDisplay and CSurface methods
//-----------------------------------------------------------------------------
#define DSURFACELOCK_READ
#define DSURFACELOCK_WRITE




//-----------------------------------------------------------------------------
// Name: class CDisplay
// Desc: Class to handle all DDraw aspects of a display, including creation of
//       front and back buffers, creating offscreen surfaces and palettes,
//       and blitting surface and displaying bitmaps.
//-----------------------------------------------------------------------------
class CDisplay
{
public:
    LPDIRECTDRAW7        m_pDD;
    LPDIRECTDRAWSURFACE7 m_pddsFrontBuffer;
    LPDIRECTDRAWSURFACE7 m_pddsBackBuffer;
    LPDIRECTDRAWSURFACE7 m_pddsBackBufferLeft; // For stereo modes

    HWND                 m_hWnd;
    RECT                 m_rcWindow;
    BOOL                 m_bWindowed;
    BOOL                 m_bStereo;

public:
    CDisplay();
    ~CDisplay();

    // Access functions
	BOOL	             GetCaps(LPDDCAPS caps);   
    HWND                 GetHWnd()           { return m_hWnd; }
	int					 GetWidth()			 { return m_rcWindow.right-m_rcWindow.left; };
	int					 GetHeight()		 { return m_rcWindow.bottom-m_rcWindow.top; };
    LPDIRECTDRAW7        GetDirectDraw()     { return m_pDD; }
    LPDIRECTDRAWSURFACE7 GetFrontBuffer()    { return m_pddsFrontBuffer; }
    LPDIRECTDRAWSURFACE7 GetBackBuffer()     { return m_pddsBackBuffer; }
    LPDIRECTDRAWSURFACE7 GetBackBufferLEft() { return m_pddsBackBufferLeft; }
	DWORD				 GetMonitorFrequency() { DWORD x; m_pDD->GetMonitorFrequency(&x); return x;};

    // Status functions
    BOOL				 IsWindowed()        { return m_bWindowed; }
    BOOL				 IsStereo()          { return m_bStereo; }
	BOOL				 m_bVsync;			// only for fullscreen flips
	static BOOL			 m_bFirst;

    // Creation/destruction methods
    HRESULT CreateFullScreenDisplay( HWND hWnd, DWORD dwWidth, DWORD dwHeight, DWORD dwBPP, GUID *pGuid = NULL);
    HRESULT CreateWindowedDisplay( HWND hWnd, DWORD dwWidth, DWORD dwHeight, GUID *pGuid = NULL);
    HRESULT InitClipper();
    HRESULT UpdateBounds();
    virtual HRESULT DestroyObjects();

    // Methods to create child objects
    HRESULT CreateSurface( CSurface** ppSurface, DWORD dwWidth, DWORD dwHeight );
    HRESULT CreateSurfaceFromBitmap( CSurface** ppSurface, LPCWSTR strBMP, DWORD dwDesiredWidth, DWORD dwDesiredHeight );
    HRESULT CreateSurfaceFromText( CSurface** ppSurface, HFONT hFont, wchar_t * strText, COLORREF crBackground, COLORREF crForeground, BOOL bMultiLine=false);
    HRESULT CreatePaletteFromBitmap( LPDIRECTDRAWPALETTE* ppPalette, const TCHAR* strBMP );

    // Display methods
    HRESULT Clear( DWORD dwColor = 0L );
    HRESULT ClearRect( RECT * r, DWORD dwColor = 0L );
    HRESULT ClearFront( DWORD dwColor = 0L );
    HRESULT ColorKeyBlt( DWORD x, DWORD y, LPDIRECTDRAWSURFACE7 pdds, RECT* prc = NULL );
    HRESULT Blt( DWORD x, DWORD y, LPDIRECTDRAWSURFACE7 pdds, RECT* prc=NULL, DWORD dwFlags=0 );
    HRESULT Blt( DWORD x, DWORD y, CSurface* pSurface, RECT* prc = NULL );
    HRESULT ShowBitmap( HBITMAP hbm, LPDIRECTDRAWPALETTE pPalette=NULL );
    HRESULT SetPalette( LPDIRECTDRAWPALETTE pPalette );
	DWORD ConvertGDIColor( COLORREF dwGDIColor );
	HRESULT Present();
};




//-----------------------------------------------------------------------------
// Name: class CSurface
// Desc: Class to handle aspects of a DirectDrawSurface.
//-----------------------------------------------------------------------------
class CSurface
{
    LPDIRECTDRAWSURFACE7 m_pdds;
    DDSURFACEDESC2       m_ddsd;
    BOOL                 m_bColorKeyed;

public:
    LPDIRECTDRAWSURFACE7 GetDDrawSurface()	{ return m_pdds; }
	LPDIRECTDRAWSURFACE7 GetDDS( void )		{ return m_pdds; }
    DDSURFACEDESC2 	   * GetSurfaceDesc()	{ return & m_ddsd; }
    BOOL                 IsColorKeyed()		{ return m_bColorKeyed; }
	inline int			 GetWidth(){return m_ddsd.dwWidth;};
	inline int			 GetHeight(){return m_ddsd.dwHeight;};


    HRESULT DrawSurface( CSurface * surface, DWORD dstX, DWORD dstY);
    HRESULT DrawBitmap( HBITMAP hBMP, DWORD dwBMPOriginX = 0, DWORD dwBMPOriginY = 0, DWORD dwBMPWidth = 0, DWORD dwBMPHeight = 0 );
    HRESULT DrawBitmap( TCHAR* strBMP, DWORD dwDesiredWidth, DWORD dwDesiredHeight );
    HRESULT DrawText( HFONT hFont, TCHAR* strText, DWORD dwOriginX, DWORD dwOriginY, COLORREF crBackground, COLORREF crForeground,BOOL bMultiLine=false );


    HRESULT SetColorKey( DWORD dwColorKey );
    DWORD   ConvertGDIColor( COLORREF dwGDIColor );
    static HRESULT GetBitMaskInfo( DWORD dwBitMask, DWORD* pdwShift, DWORD* pdwBits );

    HRESULT Create( LPDIRECTDRAW7 pDD, DDSURFACEDESC2* pddsd );
    HRESULT Create( LPDIRECTDRAWSURFACE7 pdds );
    HRESULT Attach( LPDIRECTDRAWSURFACE7 pdds ) {return Create(pdds);};
    HRESULT Clear( DWORD dwColor = 0L );
    HRESULT ClearRect( RECT * r, DWORD dwColor = 0L );
    HRESULT Destroy();
	HRESULT Detach();

    CSurface();
    virtual ~CSurface();
    DWORD       m_LockCount;            // Reference count to avoid mulitple lock/Unlocks


	HRESULT     Lock(void);
	HRESULT     UnLock(void);

    void*       GetSurfacePointer( void ) { return m_ddsd.lpSurface; }


	HRESULT Rectangle(int X, int Y, int W, int H, COLORREF Colref);
	HRESULT Ellipse(int X, int Y, int W, int H, COLORREF Colref, bool bFilled);
	HRESULT Arc(int X1, int Y1, int X2, int Y2, int startX, int startY, int endX, int endY, COLORREF Colref);
	HRESULT Line(int X1, int Y1, int X2, int Y2, COLORREF Colref);
    HRESULT PutPixel8(int X, int Y, DWORD Col);
    HRESULT PutPixel16(int X, int Y, DWORD Col);
    HRESULT PutPixel24(int X, int Y, DWORD Col);
    HRESULT PutPixel32(int X, int Y, DWORD Col);


    HRESULT VLine8(int Y1, int Y2, int X, DWORD Col);
    HRESULT VLine16(int Y1, int Y2, int X, DWORD Col);
    HRESULT VLine24(int Y1, int Y2, int X, DWORD Col);
    HRESULT VLine32(int Y1, int Y2, int X, DWORD Col);

    HRESULT HLine8(int Y1, int Y2, int X, DWORD Col);
    HRESULT HLine16(int Y1, int Y2, int X, DWORD Col);
    HRESULT HLine24(int Y1, int Y2, int X, DWORD Col);
    HRESULT HLine32(int Y1, int Y2, int X, DWORD Col);

    inline HRESULT PutPixel(int X, int Y, DWORD Col) { return (*this.*m_fpPutPixel)(X, Y, Col); }
    HRESULT (CSurface::* m_fpPutPixel)(int X, int Y, DWORD Col);
	HRESULT (CSurface::* m_fpVLine)(int Y1, int Y2, int X, DWORD Col);
	HRESULT (CSurface::* m_fpHLine)(int X1, int X2, int Y, DWORD Col);

    inline HRESULT VLine(int Y1, int Y2, int X, DWORD Col) { return (*this.*m_fpVLine)(Y1, Y2, X, Col); }
    inline HRESULT HLine(int X1, int X2, int Y, DWORD Col) { return (*this.*m_fpHLine)(X1, X2, Y, Col); }
	void FunctionMapper();

	
	static void Swap(int *a, int *b) {int Temp = *a;*a = *b;*b = Temp;};

	HRESULT GetDC(HDC & theDC);
	HRESULT ReleaseDC(HDC theDC);

};




#endif // DDUTIL_H

