//-----------------------------------------------------------------------------
// File: ddutil.cpp
//
// Desc: DirectDraw framewark classes. Feel free to use this class as a 
//       starting point for adding extra functionality.
//
//
// Copyright (c) 1995-1999 Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#define STRICT
#include <tchar.h>
#include <windows.h>
#include <windowsx.h>
#include <ddraw.h>
#include <stdio.h>
#include "ddutil.h"
#include "dxutil.h"


BOOL CDisplay::m_bFirst = true;


HRESULT CSurface::GetDC(HDC &theDC) 
{
	HRESULT hr;
    if( FAILED( hr = m_pdds->Restore() ) )
        return hr;
	return m_pdds->GetDC( &theDC ); 
}
HRESULT CSurface::ReleaseDC(HDC theDC) 
{
	return m_pdds->ReleaseDC( theDC ); 
};



//-----------------------------------------------------------------------------
// Name: CDisplay()
// Desc:
//-----------------------------------------------------------------------------
CDisplay::CDisplay()
{
    m_pDD                = NULL;
    m_pddsFrontBuffer    = NULL;
    m_pddsBackBuffer     = NULL;
    m_pddsBackBufferLeft = NULL;
	m_bVsync			 = FALSE;
}




//-----------------------------------------------------------------------------
// Name: ~CDisplay()
// Desc:
//-----------------------------------------------------------------------------
CDisplay::~CDisplay()
{
    DestroyObjects();
}




//-----------------------------------------------------------------------------
// Name: DestroyObjects()
// Desc:
//-----------------------------------------------------------------------------
HRESULT CDisplay::DestroyObjects()
{
    SAFE_RELEASE( m_pddsBackBufferLeft );
    SAFE_RELEASE( m_pddsBackBuffer ); 
    SAFE_RELEASE( m_pddsFrontBuffer );

    if( m_pDD ) 
	{
        m_pDD->SetCooperativeLevel( m_hWnd, DDSCL_NORMAL );
		m_pDD->RestoreDisplayMode();
		m_pDD->Release();

	}

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: GetCaps(LPDDCAPS)
// Desc:
//-----------------------------------------------------------------------------
BOOL CDisplay::GetCaps(LPDDCAPS caps)
{
	return (m_pDD->GetCaps(caps, NULL)==DD_OK);
}



//-----------------------------------------------------------------------------
// Name: CreateFullScreenDisplay()
// Desc:
//-----------------------------------------------------------------------------
HRESULT CDisplay::CreateFullScreenDisplay( HWND hWnd, DWORD dwWidth,
                                           DWORD dwHeight, DWORD dwBPP , GUID *pGuid)
{
    HRESULT				hr;
	DWORD               dwFlags;

    // Cleanup anything from a previous call
    DestroyObjects();

    // DDraw stuff begins here
    if( FAILED( hr = DirectDrawCreateEx( pGuid, (VOID**)&m_pDD,
                                         IID_IDirectDraw7, NULL ) ) )
        return E_FAIL;

    // Set cooperative level
	if (m_bFirst)
	{
        dwFlags = DDSCL_SETFOCUSWINDOW;
        if( FAILED( hr = m_pDD->SetCooperativeLevel( hWnd, dwFlags ) ) )
            return hr;

        dwFlags = DDSCL_ALLOWREBOOT | DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN;
        if( FAILED( hr = m_pDD->SetCooperativeLevel(hWnd, dwFlags ) ) )
            return hr;
	}
	else
	{
            dwFlags = DDSCL_SETFOCUSWINDOW | DDSCL_CREATEDEVICEWINDOW |
                      DDSCL_ALLOWREBOOT | DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN;
            if( FAILED( hr = m_pDD->SetCooperativeLevel(hWnd, dwFlags ) ) )
                return hr;
	}


    // Set the display mode
    if( FAILED( m_pDD->SetDisplayMode( dwWidth, dwHeight, dwBPP, 0, 0 ) ) )
        return E_FAIL;

    // Create primary surface (with backbuffer attached)
    DDSURFACEDESC2 ddsd;
    ZeroMemory( &ddsd, sizeof( ddsd ) );
    ddsd.dwSize            = sizeof( ddsd );
    ddsd.dwFlags           = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
    ddsd.ddsCaps.dwCaps    = DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP |
                             DDSCAPS_COMPLEX | DDSCAPS_3DDEVICE | DDSCAPS_VIDEOMEMORY;
    ddsd.dwBackBufferCount = 1;

    if( FAILED( hr = m_pDD->CreateSurface( &ddsd, &m_pddsFrontBuffer, NULL ) ) )
        return E_FAIL;

    // Get a pointer to the back buffer
    DDSCAPS2 ddscaps;
    ZeroMemory( &ddscaps, sizeof( ddscaps ) );
    ddscaps.dwCaps = DDSCAPS_BACKBUFFER;

    if( FAILED( hr = m_pddsFrontBuffer->GetAttachedSurface( &ddscaps, &m_pddsBackBuffer ) ) )
        return E_FAIL;

    m_pddsBackBuffer->AddRef();

    m_hWnd      = hWnd;
    m_bWindowed = FALSE;
    UpdateBounds();

    return S_OK;
}
    



//-----------------------------------------------------------------------------
// Name: CreateWindowedDisplay()
// Desc:
//-----------------------------------------------------------------------------
HRESULT CDisplay::CreateWindowedDisplay( HWND hWnd, DWORD dwWidth, DWORD dwHeight, GUID *pGuid)
{
    HRESULT hr;

    // Cleanup anything from a previous call
    DestroyObjects();

    // DDraw stuff begins here
	// THIS STILL NEEDS OPTIMISATION FOR MULTIMON
	// STILL TO DO !!!!!!! VERY IMPORTANT !!!!!!!!!!!!!!!
    if( FAILED( hr = DirectDrawCreateEx( pGuid, (VOID**)&m_pDD,
                                         IID_IDirectDraw7, NULL ) ) )
        return E_FAIL;

    // Set cooperative level
    hr = m_pDD->SetCooperativeLevel( hWnd, DDSCL_NORMAL );
    if( FAILED(hr) )
        return E_FAIL;

    RECT  rcWork;
    RECT  rc;
    DWORD dwStyle;

    // If we are still a WS_POPUP window we should convert to a normal app
    // window so we look like a windows app.
    dwStyle  = GetWindowStyle( hWnd );
    dwStyle &= ~WS_POPUP;
    dwStyle |= WS_OVERLAPPED | WS_CAPTION | WS_THICKFRAME | WS_MINIMIZEBOX;
    SetWindowLong( hWnd, GWL_STYLE, dwStyle );

    // Aet window size
    SetRect( &rc, 0, 0, dwWidth, dwHeight );

    AdjustWindowRectEx( &rc, GetWindowStyle(hWnd), GetMenu(hWnd) != NULL,
                        GetWindowExStyle(hWnd) );

    SetWindowPos( hWnd, NULL, 0, 0, rc.right-rc.left, rc.bottom-rc.top,
                  SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE );

    SetWindowPos( hWnd, HWND_NOTOPMOST, 0, 0, 0, 0,
                  SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE );

    //  Make sure our window does not hang outside of the work area
    SystemParametersInfo( SPI_GETWORKAREA, 0, &rcWork, 0 );
    GetWindowRect( hWnd, &rc );
    if( rc.left < rcWork.left ) rc.left = rcWork.left;
    if( rc.top  < rcWork.top )  rc.top  = rcWork.top;
    SetWindowPos( hWnd, NULL, rc.left, rc.top, 0, 0,
                  SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE );

    LPDIRECTDRAWCLIPPER pcClipper;
    
    // Create the primary surface
    DDSURFACEDESC2 ddsd;
    ZeroMemory( &ddsd, sizeof( ddsd ) );
    ddsd.dwSize         = sizeof( ddsd );
    ddsd.dwFlags        = DDSD_CAPS;
    ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_VIDEOMEMORY;

    if( FAILED( m_pDD->CreateSurface( &ddsd, &m_pddsFrontBuffer, NULL ) ) )
        return E_FAIL;

    // Create the backbuffer surface
    ddsd.dwFlags        = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;    
    ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_3DDEVICE | DDSCAPS_VIDEOMEMORY;
    ddsd.dwWidth        = dwWidth;
    ddsd.dwHeight       = dwHeight;

    if( FAILED( hr = m_pDD->CreateSurface( &ddsd, &m_pddsBackBuffer, NULL ) ) )
        return E_FAIL;

    if( FAILED( hr = m_pDD->CreateClipper( 0, &pcClipper, NULL ) ) )
        return E_FAIL;

    if( FAILED( hr = pcClipper->SetHWnd( 0, hWnd ) ) )
    {
        pcClipper->Release();
        return E_FAIL;
    }

    if( FAILED( hr = m_pddsFrontBuffer->SetClipper( pcClipper ) ) )
    {
        pcClipper->Release();
        return E_FAIL;
    }

    // Done with clipper
    pcClipper->Release();

    m_hWnd      = hWnd;
    m_bWindowed = TRUE;
    UpdateBounds();

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
HRESULT CDisplay::CreateSurface( CSurface** ppSurface,
                                 DWORD dwWidth, DWORD dwHeight )
{
    if( NULL == m_pDD )
        return E_POINTER;
    if( NULL == ppSurface )
        return E_INVALIDARG;

    HRESULT        hr;
    DDSURFACEDESC2 ddsd;
    ZeroMemory( &ddsd, sizeof( ddsd ) );
    ddsd.dwSize         = sizeof( ddsd );
    ddsd.dwFlags        = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT; 
    ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_VIDEOMEMORY;
    ddsd.dwWidth        = dwWidth;
    ddsd.dwHeight       = dwHeight;

    (*ppSurface) = new CSurface();
    if( FAILED( hr = (*ppSurface)->Create( m_pDD, &ddsd ) ) )
    {
        delete (*ppSurface);
        return hr;
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: CDisplay::CreateSurfaceFromBitmap()
// Desc: Create a DirectDrawSurface from a bitmap resource or bitmap file.
//       Use MAKEINTRESOURCE() to pass a constant into strBMP.
//-----------------------------------------------------------------------------
HRESULT CDisplay::CreateSurfaceFromBitmap( CSurface** ppSurface,
                                           LPCWSTR strBMP,                                            
                                           DWORD dwDesiredWidth, 
                                           DWORD dwDesiredHeight )
{
    HRESULT        hr;
    HBITMAP        hBMP = NULL;
    BITMAP         bmp;
    DDSURFACEDESC2 ddsd;

    if( m_pDD == NULL || strBMP == NULL || ppSurface == NULL ) 
        return E_INVALIDARG;

    *ppSurface = NULL;

    //  Try to load the bitmap as a resource, if that fails, try it as a file
    hBMP = (HBITMAP) LoadImage( GetModuleHandle(NULL), strBMP, 
                                IMAGE_BITMAP, dwDesiredWidth, dwDesiredHeight, 
                                LR_CREATEDIBSECTION );
    if( hBMP == NULL )
    {
        hBMP = (HBITMAP) LoadImage( NULL, strBMP, 
                                    IMAGE_BITMAP, dwDesiredWidth, dwDesiredHeight, 
                                    LR_LOADFROMFILE | LR_CREATEDIBSECTION );
        if( hBMP == NULL )
            return E_FAIL;
    }

    // Get size of the bitmap
    GetObject( hBMP, sizeof(bmp), &bmp );

    // Create a DirectDrawSurface for this bitmap
    ZeroMemory( &ddsd, sizeof(ddsd) );
    ddsd.dwSize         = sizeof(ddsd);
    ddsd.dwFlags        = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
    ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_VIDEOMEMORY;
    ddsd.dwWidth        = bmp.bmWidth;
    ddsd.dwHeight       = bmp.bmHeight;

    (*ppSurface) = new CSurface();
    if( FAILED( hr = (*ppSurface)->Create( m_pDD, &ddsd ) ) )
    {
        delete (*ppSurface);
        return hr;
    }

    // Draw the bitmap on this surface
    if( FAILED( hr = (*ppSurface)->DrawBitmap( hBMP, 0, 0, 0, 0 ) ) )
    {
        DeleteObject( hBMP );
        return hr;
    }

    DeleteObject( hBMP );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: CDisplay::CreateSurfaceFromText()
// Desc: Creates a DirectDrawSurface from a text string using hFont or the default 
//       GDI font if hFont is NULL.
//-----------------------------------------------------------------------------
HRESULT CDisplay::CreateSurfaceFromText( CSurface** ppSurface,
                                         HFONT hFont, wchar_t * strText, 
                                         COLORREF crBackground, COLORREF crForeground, 
										 BOOL bMultiLine )
{
    HDC                  hDC  = NULL;
    LPDIRECTDRAWSURFACE7 pDDS = NULL;
    HRESULT              hr;
    DDSURFACEDESC2       ddsd;
    SIZE                 sizeText;

    if( m_pDD == NULL || strText == NULL || ppSurface == NULL )
        return E_INVALIDARG;

    *ppSurface = NULL;

    hDC = GetDC( NULL );

    if( hFont )
        SelectObject( hDC, hFont );

	if (!bMultiLine)
	    GetTextExtentPoint32( hDC, strText, _tcslen(strText), &sizeText ); // THIS WORKS ONLY FOR SINGLE-LINE TEXT !
	else
	{
		RECT rect;
		rect.top=rect.left=0;
		rect.bottom=10;
		rect.right=800;
		::DrawText(hDC, strText, _tcslen(strText), &rect, DT_CALCRECT|DT_WORDBREAK);
		sizeText.cx=rect.right;
		sizeText.cy=rect.bottom;
	}
    ReleaseDC( NULL, hDC );

    // Create a DirectDrawSurface for this bitmap
    ZeroMemory( &ddsd, sizeof(ddsd) );
    ddsd.dwSize         = sizeof(ddsd);
    ddsd.dwFlags        = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
    ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_VIDEOMEMORY;
    ddsd.dwWidth        = sizeText.cx;
    ddsd.dwHeight       = sizeText.cy;

    (*ppSurface) = new CSurface();
    if( FAILED( hr = (*ppSurface)->Create( m_pDD, &ddsd ) ) )
    {
        delete (*ppSurface);
        return hr;
    }

	(*ppSurface)->Clear(crBackground);

    if( FAILED( hr = (*ppSurface)->DrawText( hFont, strText, 0, 0, 
                                             crBackground, crForeground, bMultiLine ) ) )
        return hr;

    return S_OK;
}



//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
HRESULT CDisplay::Present()
{
    HRESULT hr;

    if( NULL == m_pddsFrontBuffer && NULL == m_pddsBackBuffer )
        return E_POINTER;

    while( 1 )
    {
        if( m_bWindowed )
            hr = m_pddsFrontBuffer->Blt( &m_rcWindow, m_pddsBackBuffer,
                                         NULL, DDBLT_WAIT, NULL );
        else
		{
			
            hr = m_pddsFrontBuffer->Flip( NULL, DDFLIP_NOVSYNC); //m_bVsync?NULL:
		}

        if( hr == DDERR_SURFACELOST )
        {
            m_pddsFrontBuffer->Restore();
            m_pddsBackBuffer->Restore();
        }

        if( hr != DDERR_WASSTILLDRAWING )
            return hr;
    }
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
HRESULT CDisplay::ShowBitmap( HBITMAP hbm, LPDIRECTDRAWPALETTE pPalette )
{
    if( NULL == m_pddsFrontBuffer ||  NULL == m_pddsBackBuffer )
        return E_POINTER;

    // Set the palette before loading the bitmap
    if( pPalette )
        m_pddsFrontBuffer->SetPalette( pPalette );

    CSurface backBuffer;
    backBuffer.Create( m_pddsBackBuffer );

    if( FAILED( backBuffer.DrawBitmap( hbm, 0, 0, 0, 0 ) ) )
        return E_FAIL;

    return Present();
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
HRESULT CDisplay::ColorKeyBlt( DWORD x, DWORD y, LPDIRECTDRAWSURFACE7 pdds,
                               RECT* prc )
{
    if( NULL == m_pddsBackBuffer )
        return E_POINTER;

    return m_pddsBackBuffer->BltFast( x, y, pdds, prc, DDBLTFAST_SRCCOLORKEY );
}





//-----------------------------------------------------------------------------
// Name: CDisplay::ConvertGDIColor()
// Desc: Converts a GDI color (0x00bbggrr) into the equivalent color on a 
//       DirectDrawSurface using its pixel format.  
//-----------------------------------------------------------------------------
DWORD CDisplay::ConvertGDIColor( COLORREF dwGDIColor )
{
    DDPIXELFORMAT   pf;
    unsigned int    i, j, 
                    rshift, gshift, bshift, 
                    rbits, gbits, bbits,
                    RED , GREEN , BLUE,
                    result;
    
    // separate red,green.blue out of Color
    RED   = dwGDIColor & 255;
    GREEN = ( dwGDIColor >> 8 ) & 255;
    BLUE  = ( dwGDIColor >> 16 ) & 255;
 

    // get the destination surface pixel format
    ZeroMemory( &pf , sizeof( pf ) );
    pf.dwSize = sizeof(pf);

	if (m_pddsBackBuffer != NULL)
		m_pddsBackBuffer->GetPixelFormat( &pf );
	else if (m_pddsBackBuffer != NULL)
		m_pddsFrontBuffer->GetPixelFormat( &pf );
	else
		return 0;

    // convert the color
    if (pf.dwRGBBitCount>8)
    {
        j = (int) pf.dwRBitMask; rshift = 0;
        i = 1; while (!(i&j)) { rshift++; i<<=1; }
        rbits = 0; while (i&j) { rbits++; i<<=1; }
        j = (int) pf.dwGBitMask; gshift = 0;
        i = 1; while (!(i&j)) { gshift++; i<<=1; }
        gbits = 0; while (i&j) { gbits++; i<<=1; }
        j = (int) pf.dwBBitMask; bshift = 0;
        i = 1; while (!(i&j)) { bshift++; i<<=1; }
        bbits = 0; while (i&j) { bbits++; i<<=1; }

        result = (((RED<<rshift)>>(8-rbits)) & pf.dwRBitMask) |
                 (((GREEN<<gshift)>>(8-gbits)) & pf.dwGBitMask) |
                 (((BLUE<<bshift)>>(8-bbits)) & pf.dwBBitMask);
    }
    else
    {
        result = dwGDIColor;
    }

return result;


    if( m_pddsFrontBuffer == NULL )
	    return 0x00000000;

    COLORREF       rgbT;
    HDC            hdc;
    DWORD          dw = CLR_INVALID;
    DDSURFACEDESC2 ddsd;
    HRESULT        hr;

    //  Use GDI SetPixel to color match for us
    if( dwGDIColor != CLR_INVALID && m_pddsFrontBuffer->GetDC(&hdc) == DD_OK)
    {
        rgbT = GetPixel(hdc, 0, 0);     // Save current pixel value
        SetPixel(hdc, 0, 0, dwGDIColor);       // Set our value
        m_pddsFrontBuffer->ReleaseDC(hdc);
    }

    // Now lock the surface so we can read back the converted color
    ddsd.dwSize = sizeof(ddsd);
    hr = m_pddsFrontBuffer->Lock( NULL, &ddsd, DDLOCK_WAIT, NULL );
    if( hr == DD_OK)
    {
        dw = *(DWORD *) ddsd.lpSurface; 
        if( ddsd.ddpfPixelFormat.dwRGBBitCount < 32 ) // Mask it to bpp
            dw &= ( 1 << ddsd.ddpfPixelFormat.dwRGBBitCount ) - 1;  
        m_pddsFrontBuffer->Unlock(NULL);
    }

    //  Now put the color that was there back.
    if( dwGDIColor != CLR_INVALID && m_pddsFrontBuffer->GetDC(&hdc) == DD_OK )
    {
        SetPixel( hdc, 0, 0, rgbT );
        m_pddsFrontBuffer->ReleaseDC(hdc);
    }
    
    return dw;    
}



//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
HRESULT CDisplay::Blt( DWORD x, DWORD y, LPDIRECTDRAWSURFACE7 pdds, RECT* prc,
                       DWORD dwFlags )
{
    if( NULL == m_pddsBackBuffer )
        return E_POINTER;

    HRESULT res = m_pddsBackBuffer->BltFast( x, y, pdds, prc, dwFlags );
    
	if (res == DDERR_EXCEPTION)
		return res;
	if (res == DDERR_GENERIC)
		return res;
	if (res == DDERR_INVALIDOBJECT)
		return res;
	if (res == DDERR_INVALIDPARAMS)
		return res;
	if (res == DDERR_INVALIDRECT)
		return res;
	if (res == DDERR_NOBLTHW)
		return res;
	if (res == DDERR_SURFACEBUSY)
		return res;
	if (res == DDERR_SURFACELOST)
		return res;
	if (res == DDERR_UNSUPPORTED)
		return res;
	if (res == DDERR_WASSTILLDRAWING)
		return res;
	else
		return res;
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
HRESULT CDisplay::Blt( DWORD x, DWORD y, CSurface* pSurface, RECT* prc )
{
    if( NULL == pSurface )
        return E_INVALIDARG;

    if( pSurface->IsColorKeyed() )
        return Blt( x, y, pSurface->GetDDrawSurface(), prc, DDBLTFAST_SRCCOLORKEY );
    else
        return Blt( x, y, pSurface->GetDDrawSurface(), prc, 0L );
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
HRESULT CDisplay::Clear( DWORD dwColor )
{
    if( NULL == m_pddsBackBuffer )
        return E_POINTER;

    // Erase the background
    DDBLTFX ddbltfx;
    ZeroMemory( &ddbltfx, sizeof(ddbltfx) );
    ddbltfx.dwSize      = sizeof(ddbltfx);
    ddbltfx.dwFillColor = dwColor;

    return m_pddsBackBuffer->Blt( NULL, NULL, NULL, DDBLT_COLORFILL, &ddbltfx );
}


//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
HRESULT CDisplay::ClearRect( RECT * r, DWORD dwColor )
{
    if( NULL == m_pddsBackBuffer )
        return E_POINTER;

    // Erase the background
    DDBLTFX ddbltfx;
    ZeroMemory( &ddbltfx, sizeof(ddbltfx) );
    ddbltfx.dwSize      = sizeof(ddbltfx);
    ddbltfx.dwFillColor = dwColor;

    return m_pddsBackBuffer->Blt( r, NULL, NULL, DDBLT_COLORFILL, &ddbltfx );
}


//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
HRESULT CDisplay::ClearFront( DWORD dwColor )
{
    if( NULL == m_pddsFrontBuffer )
        return E_POINTER;

    // Erase the background
    DDBLTFX ddbltfx;
    ZeroMemory( &ddbltfx, sizeof(ddbltfx) );
    ddbltfx.dwSize      = sizeof(ddbltfx);
    ddbltfx.dwFillColor = dwColor;

    return m_pddsFrontBuffer->Blt( NULL, NULL, NULL, DDBLT_COLORFILL, &ddbltfx );
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
HRESULT CDisplay::SetPalette( LPDIRECTDRAWPALETTE pPalette )
{
    if( NULL == m_pddsFrontBuffer )
        return E_POINTER;

    return m_pddsFrontBuffer->SetPalette( pPalette );
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
HRESULT CDisplay::CreatePaletteFromBitmap( LPDIRECTDRAWPALETTE* ppPalette,
                                           const TCHAR* strBMP )
{
    HRSRC             hResource      = NULL;
    RGBQUAD*          pRGB           = NULL;
    BITMAPINFOHEADER* pbi = NULL;
    PALETTEENTRY      aPalette[256];
    HANDLE            hFile = NULL;
    DWORD             iColor;
    DWORD             dwColors;
    BITMAPFILEHEADER  bf;
    BITMAPINFOHEADER  bi;
    DWORD             dwBytesRead;

    if( m_pDD == NULL || strBMP == NULL || ppPalette == NULL )
        return E_INVALIDARG;

    *ppPalette = NULL;

    //  Try to load the bitmap as a resource, if that fails, try it as a file
    hResource = FindResource( NULL, strBMP, RT_BITMAP );
    if( hResource )
    {
        pbi = (LPBITMAPINFOHEADER) LockResource( LoadResource( NULL, hResource ) );       
        if( NULL == pbi )
            return E_FAIL;

        pRGB = (RGBQUAD*) ( (BYTE*) pbi + pbi->biSize );

        // Figure out how many colors there are
        if( pbi == NULL || pbi->biSize < sizeof(BITMAPINFOHEADER) )
            dwColors = 0;
        else if( pbi->biBitCount > 8 )
            dwColors = 0;
        else if( pbi->biClrUsed == 0 )
            dwColors = 1 << pbi->biBitCount;
        else
            dwColors = pbi->biClrUsed;

        //  A DIB color table has its colors stored BGR not RGB
        //  so flip them around.
        for( iColor = 0; iColor < dwColors; iColor++ )
        {
            aPalette[iColor].peRed   = pRGB[iColor].rgbRed;
            aPalette[iColor].peGreen = pRGB[iColor].rgbGreen;
            aPalette[iColor].peBlue  = pRGB[iColor].rgbBlue;
            aPalette[iColor].peFlags = 0;
        }

        return m_pDD->CreatePalette( DDPCAPS_8BIT, aPalette, ppPalette, NULL );
    }

    // Attempt to load bitmap as a file
    hFile = CreateFile( strBMP, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL );
    if( NULL == hFile )
        return E_FAIL;

    // Read the BITMAPFILEHEADER
    ReadFile( hFile, &bf, sizeof(bf), &dwBytesRead, NULL );
    if( dwBytesRead != sizeof(bf) )
    {
        CloseHandle( hFile );
        return E_FAIL;
    }

    // Read the BITMAPINFOHEADER
    ReadFile( hFile, &bi, sizeof(bi), &dwBytesRead, NULL );
    if( dwBytesRead != sizeof(bi) )
    {
        CloseHandle( hFile );
        return E_FAIL;
    }

    // Read the PALETTEENTRY 
    ReadFile( hFile, aPalette, sizeof(aPalette), &dwBytesRead, NULL );
    if( dwBytesRead != sizeof(aPalette) )
    {
        CloseHandle( hFile );
        return E_FAIL;
    }

    CloseHandle( hFile );

    // Figure out how many colors there are
    if( bi.biSize != sizeof(BITMAPINFOHEADER) )
        dwColors = 0;
    else if (bi.biBitCount > 8)
        dwColors = 0;
    else if (bi.biClrUsed == 0)
        dwColors = 1 << bi.biBitCount;
    else
        dwColors = bi.biClrUsed;

    //  A DIB color table has its colors stored BGR not RGB
    //  so flip them around since DirectDraw uses RGB
    for( iColor = 0; iColor < dwColors; iColor++ )
    {
        BYTE r = aPalette[iColor].peRed;
        aPalette[iColor].peRed  = aPalette[iColor].peBlue;
        aPalette[iColor].peBlue = r;
    }

    return m_pDD->CreatePalette( DDPCAPS_8BIT, aPalette, ppPalette, NULL );
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
HRESULT CDisplay::UpdateBounds()
{
    if( m_bWindowed )
    {
        GetClientRect( m_hWnd, &m_rcWindow );
        ClientToScreen( m_hWnd, (POINT*)&m_rcWindow );
        ClientToScreen( m_hWnd, (POINT*)&m_rcWindow+1 );
    }
    else
    {
        SetRect( &m_rcWindow, 0, 0, GetSystemMetrics(SM_CXSCREEN),
                 GetSystemMetrics(SM_CYSCREEN) );
    }

    return S_OK;
}





//-----------------------------------------------------------------------------
// Name: CDisplay::InitClipper
// Desc: 
//-----------------------------------------------------------------------------
HRESULT CDisplay::InitClipper()
{
    LPDIRECTDRAWCLIPPER pClipper;
    HRESULT hr;

    // Create a clipper when using GDI to draw on the primary surface 
    if( FAILED( hr = m_pDD->CreateClipper( 0, &pClipper, NULL ) ) )
        return hr;

    pClipper->SetHWnd( 0, m_hWnd );

    if( FAILED( hr = m_pddsFrontBuffer->SetClipper( pClipper ) ) )
        return hr;

    // We can release the clipper now since g_pDDSPrimary 
    // now maintains a ref count on the clipper
    SAFE_RELEASE( pClipper );

    return S_OK;
}





//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
CSurface::CSurface()
{
    m_LockCount         = 0;
    m_pdds				= NULL;
    m_bColorKeyed		= NULL;
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
CSurface::~CSurface()
{
    SAFE_RELEASE( m_pdds );
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
HRESULT CSurface::Create( LPDIRECTDRAWSURFACE7 pdds )
{
    m_pdds = pdds;

    if( m_pdds )
    {
        m_pdds->AddRef();

        // Get the DDSURFACEDESC structure for this surface
        m_ddsd.dwSize = sizeof(m_ddsd);
        m_pdds->GetSurfaceDesc( &m_ddsd );
    }

    FunctionMapper();
    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
HRESULT CSurface::Create( LPDIRECTDRAW7 pDD, DDSURFACEDESC2* pddsd )
{
    HRESULT hr;

    // Create the DDraw surface
    if( FAILED( hr = pDD->CreateSurface( pddsd, &m_pdds, NULL ) ) )
        return hr;

    // Prepare the DDSURFACEDESC structure
    m_ddsd.dwSize = sizeof(m_ddsd);

    // Get the DDSURFACEDESC structure for this surface
    m_pdds->GetSurfaceDesc( &m_ddsd );

    FunctionMapper();
    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
HRESULT CSurface::Destroy()
{
    SAFE_RELEASE( m_pdds );
    return S_OK;
}

HRESULT CSurface::Detach()
{
	m_pdds->Release();
	m_pdds = NULL;
    return S_OK;
}



//-----------------------------------------------------------------------------
// Name: CSurface::DrawSurface()
// Desc: Draws a given surface
//-----------------------------------------------------------------------------
HRESULT CSurface::DrawSurface(CSurface * surf, 
                              DWORD dstX, DWORD dstY)
{
	if (!surf)
		return -1;

	RECT dstRect, srcRect;
	int w = surf->GetWidth();
	int h = surf->GetHeight();

	srcRect.left = srcRect.top = 0;
	srcRect.right = w;
	srcRect.bottom = h;

	dstRect.left = dstX;
	dstRect.top = dstY;
	dstRect.right = dstX + w;
	dstRect.bottom = dstY + h;

	return m_pdds->Blt(&dstRect, surf->GetDDS(), &srcRect, NULL, NULL);
}

//-----------------------------------------------------------------------------
// Name: CSurface::DrawBitmap()
// Desc: Draws a bitmap over an entire DirectDrawSurface, stretching the 
//       bitmap if nessasary
//-----------------------------------------------------------------------------
HRESULT CSurface::DrawBitmap( HBITMAP hBMP, 
                              DWORD dwBMPOriginX, DWORD dwBMPOriginY, 
                              DWORD dwBMPWidth, DWORD dwBMPHeight )
{
    HDC            hDCImage;
    HDC            hDC;
    BITMAP         bmp;
    DDSURFACEDESC2 ddsd;
    HRESULT        hr;

    if( hBMP == NULL || m_pdds == NULL )
        return E_INVALIDARG;

    // Make sure this surface is restored.
    if( FAILED( hr = m_pdds->Restore() ) )
        return hr;

    // Get the surface.description
    ddsd.dwSize  = sizeof(ddsd);
    m_pdds->GetSurfaceDesc( &ddsd );

    if( ddsd.ddpfPixelFormat.dwFlags == DDPF_FOURCC )
        return E_NOTIMPL;

    // Select bitmap into a memoryDC so we can use it.
    hDCImage = CreateCompatibleDC( NULL );
    if( NULL == hDCImage )
        return E_FAIL;

    SelectObject( hDCImage, hBMP );

    // Get size of the bitmap
    GetObject( hBMP, sizeof(bmp), &bmp );

    // Use the passed size, unless zero
    dwBMPWidth  = ( dwBMPWidth  == 0 ) ? bmp.bmWidth  : dwBMPWidth;     
    dwBMPHeight = ( dwBMPHeight == 0 ) ? bmp.bmHeight : dwBMPHeight;

    // Stretch the bitmap to cover this surface
    if( FAILED( hr = m_pdds->GetDC( &hDC ) ) )
        return hr;

    StretchBlt( hDC, 0, 0, 
                ddsd.dwWidth, ddsd.dwHeight, 
                hDCImage, dwBMPOriginX, dwBMPOriginY,
                dwBMPWidth, dwBMPHeight, SRCCOPY );

    if( FAILED( hr = m_pdds->ReleaseDC( hDC ) ) )
        return hr;

    DeleteDC( hDCImage );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: CSurface::DrawText()
// Desc: Draws a text string on a DirectDraw surface using hFont or the default
//       GDI font if hFont is NULL.  
//-----------------------------------------------------------------------------
HRESULT CSurface::DrawText( HFONT hFont, TCHAR* strText, 
                            DWORD dwOriginX, DWORD dwOriginY,
                            COLORREF crBackground, COLORREF crForeground,
							BOOL bMultiLine)
{
    HDC     hDC = NULL;
    HRESULT hr;

    if( m_pdds == NULL || strText == NULL )
        return E_INVALIDARG;

    // Make sure this surface is restored.
    if( FAILED( hr = m_pdds->Restore() ) )
        return hr;

    if( FAILED( hr = m_pdds->GetDC( &hDC ) ) )
        return hr;

    // Set the background and foreground color
    SetBkColor( hDC, crBackground );
    SetTextColor( hDC, crForeground );

    if( hFont )
        SelectObject( hDC, hFont );

    // Use GDI to draw the text on the surface
	if (bMultiLine)
	{
		RECT r;
		r.top=dwOriginY;
		r.left=dwOriginX;
		r.bottom=m_ddsd.dwHeight;
		r.right=m_ddsd.dwWidth;
		::DrawText(hDC, strText,_tcslen(strText), &r, DT_WORDBREAK);
	}
	else
	    TextOut( hDC, dwOriginX, dwOriginY, strText, _tcslen(strText) );

    if( FAILED( hr = m_pdds->ReleaseDC( hDC ) ) )
        return hr;

    return S_OK;
}



//-----------------------------------------------------------------------------
// Name: CSurface::Arc()
//-----------------------------------------------------------------------------
HRESULT CSurface::Arc(int X1, int Y1, int X2, int Y2, int startX, int startY, int endX, int endY, COLORREF Colref)
{
    HDC     hDC = NULL;
    HRESULT hr;

    if( m_pdds == NULL)
        return E_INVALIDARG;

    // Make sure this surface is restored.
    if( FAILED( hr = m_pdds->Restore() ) )
        return hr;

    if( FAILED( hr = m_pdds->GetDC( &hDC ) ) )
        return hr;

    // Set the background and foreground color
    
	HPEN p = ::CreatePen(PS_SOLID, 1, Colref);
	LOGBRUSH lb;
	lb.lbStyle = BS_HOLLOW;
	lb.lbColor = Colref;
	HBRUSH b = ::CreateBrushIndirect(&lb);

	HGDIOBJ oldp = ::SelectObject(hDC, p);
	HGDIOBJ oldb = ::SelectObject(hDC, b);
	::Arc(hDC, X1, Y1, X2,Y2,startX,startY,endX,endY);

	::SelectObject(hDC, oldp);
	::SelectObject(hDC, oldb);

	::DeleteObject(p);
	::DeleteObject(b);

    if( FAILED( hr = m_pdds->ReleaseDC( hDC ) ) )
        return hr;

    return S_OK;
}





//-----------------------------------------------------------------------------
// Name: CSurface::DrawText()
// Desc: Draws a text string on a DirectDraw surface using hFont or the default
//       GDI font if hFont is NULL.  
//-----------------------------------------------------------------------------
HRESULT CSurface::Ellipse(int X, int Y, int W, int H, COLORREF Colref, bool bFilled)
{
    HDC     hDC = NULL;
    HRESULT hr;

    if( m_pdds == NULL)
        return E_INVALIDARG;

    // Make sure this surface is restored.
    if( FAILED( hr = m_pdds->Restore() ) )
        return hr;

    if( FAILED( hr = m_pdds->GetDC( &hDC ) ) )
        return hr;

    // Set the background and foreground color
    
	HPEN p = ::CreatePen(PS_SOLID, 1, Colref);
	LOGBRUSH lb;
	if (bFilled)
		lb.lbStyle = BS_SOLID;
	else
		lb.lbStyle = BS_HOLLOW;
	lb.lbColor = Colref;
	HBRUSH b = ::CreateBrushIndirect(&lb);

	HGDIOBJ oldp = ::SelectObject(hDC, p);
	HGDIOBJ oldb = ::SelectObject(hDC, b);
	::Ellipse(hDC, X, Y, X+W, Y+H);

	::SelectObject(hDC, oldp);
	::SelectObject(hDC, oldb);

	::DeleteObject(p);
	::DeleteObject(b);

    if( FAILED( hr = m_pdds->ReleaseDC( hDC ) ) )
        return hr;

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: CSurface::ReDrawBitmapOnSurface()
// Desc: Load a bitmap from a file or resource into a DirectDraw surface.
//       normaly used to re-load a surface after a restore.
//-----------------------------------------------------------------------------
HRESULT CSurface::DrawBitmap( TCHAR* strBMP, 
                              DWORD dwDesiredWidth, DWORD dwDesiredHeight  )
{
    HBITMAP hBMP;
    HRESULT hr;

    if( m_pdds == NULL || strBMP == NULL )
        return E_INVALIDARG;

    //  Try to load the bitmap as a resource, if that fails, try it as a file
    hBMP = (HBITMAP) LoadImage( GetModuleHandle(NULL), strBMP, 
                                IMAGE_BITMAP, dwDesiredWidth, dwDesiredHeight, 
                                LR_CREATEDIBSECTION );
    if( hBMP == NULL )
    {
        hBMP = (HBITMAP) LoadImage( NULL, strBMP, IMAGE_BITMAP, 
                                    dwDesiredWidth, dwDesiredHeight, 
                                    LR_LOADFROMFILE | LR_CREATEDIBSECTION );
        if( hBMP == NULL )
            return E_FAIL;
    }

    // Draw the bitmap on this surface
    if( FAILED( hr = DrawBitmap( hBMP, 0, 0, 0, 0 ) ) )
    {
        DeleteObject( hBMP );
        return hr;
    }

    DeleteObject( hBMP );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
HRESULT CSurface::SetColorKey( DWORD dwColorKey )
{
    if( NULL == m_pdds )
        return E_POINTER;

    m_bColorKeyed = TRUE;

    DDCOLORKEY ddck;
    ddck.dwColorSpaceLowValue  = ConvertGDIColor( dwColorKey );
    ddck.dwColorSpaceHighValue = ConvertGDIColor( dwColorKey );
    
    return m_pdds->SetColorKey( DDCKEY_SRCBLT, &ddck );
}





//-----------------------------------------------------------------------------
// Name: CSurface::ConvertGDIColor()
// Desc: Converts a GDI color (0x00bbggrr) into the equivalent color on a 
//       DirectDrawSurface using its pixel format.  
//-----------------------------------------------------------------------------
DWORD CSurface::ConvertGDIColor( COLORREF dwGDIColor )
{
    if( m_pdds == NULL )
	    return 0x00000000;

    COLORREF       rgbT;
    HDC            hdc;
    DWORD          dw = CLR_INVALID;
    DDSURFACEDESC2 ddsd;
    HRESULT        hr;

    //  Use GDI SetPixel to color match for us
    if( dwGDIColor != CLR_INVALID && m_pdds->GetDC(&hdc) == DD_OK)
    {
        rgbT = GetPixel(hdc, 0, 0);     // Save current pixel value
        SetPixel(hdc, 0, 0, dwGDIColor);       // Set our value
        m_pdds->ReleaseDC(hdc);
    }

    // Now lock the surface so we can read back the converted color
    ddsd.dwSize = sizeof(ddsd);
    hr = m_pdds->Lock( NULL, &ddsd, DDLOCK_WAIT, NULL );
    if( hr == DD_OK)
    {
        dw = *(DWORD *) ddsd.lpSurface; 
        if( ddsd.ddpfPixelFormat.dwRGBBitCount < 32 ) // Mask it to bpp
            dw &= ( 1 << ddsd.ddpfPixelFormat.dwRGBBitCount ) - 1;  
        m_pdds->Unlock(NULL);
    }

    //  Now put the color that was there back.
    if( dwGDIColor != CLR_INVALID && m_pdds->GetDC(&hdc) == DD_OK )
    {
        SetPixel( hdc, 0, 0, rgbT );
        m_pdds->ReleaseDC(hdc);
    }
    
    return dw;    
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
HRESULT CSurface::ClearRect( RECT * r, DWORD dwColor )
{
    if( NULL == m_pdds )
        return E_POINTER;

    // Erase the background
    DDBLTFX ddbltfx;
    ZeroMemory( &ddbltfx, sizeof(ddbltfx) );
    ddbltfx.dwSize      = sizeof(ddbltfx);
    ddbltfx.dwFillColor = dwColor;

    return m_pdds->Blt( r, NULL, NULL, DDBLT_COLORFILL, &ddbltfx );
}



//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
HRESULT CSurface::Clear( DWORD dwColor )
{
    if( NULL == m_pdds )
        return E_POINTER;

    // Erase the background
    DDBLTFX ddbltfx;
    ZeroMemory( &ddbltfx, sizeof(ddbltfx) );
    ddbltfx.dwSize      = sizeof(ddbltfx);
    ddbltfx.dwFillColor = dwColor;

    return m_pdds->Blt( NULL, NULL, NULL, DDBLT_COLORFILL, &ddbltfx );
}




//-----------------------------------------------------------------------------
// Name: CSurface::GetBitMaskInfo()
// Desc: Returns the number of bits and the shift in the bit mask
//-----------------------------------------------------------------------------
HRESULT CSurface::GetBitMaskInfo( DWORD dwBitMask, DWORD* pdwShift, DWORD* pdwBits )
{
    DWORD dwShift = 0;
    DWORD dwBits  = 0; 

    if( pdwShift == NULL || pdwBits == NULL )
        return E_INVALIDARG;

    if( dwBitMask )
    {
        while( (dwBitMask & 1) == 0 )
        {
            dwShift++;
            dwBitMask >>= 1;
        }
    }

    while( (dwBitMask & 1) != 0 )
    {
        dwBits++;
        dwBitMask >>= 1;
    }

    *pdwShift = dwShift;
    *pdwBits  = dwBits;

    return S_OK;
}

HRESULT CSurface::Lock(void)
{
	HRESULT rval;
	
    // If the surface is already locked add 1 to ref count and return success
    if( m_LockCount > 0 )
    {
        m_LockCount++;
        return m_LockCount;
    }

	// Make sure the DX surface is valid
	if(m_pdds == NULL)
		return -1;

	ZeroMemory(&m_ddsd, sizeof(m_ddsd));
	m_ddsd.dwSize = sizeof(m_ddsd);

	rval = GetDDS()->Lock(NULL, &m_ddsd, DDLOCK_WAIT, NULL);

	while(rval == DDERR_SURFACELOST) 
    {
		GetDDS()->Restore();
		ZeroMemory(&m_ddsd, sizeof(m_ddsd));
		m_ddsd.dwSize = sizeof(m_ddsd);
		rval = GetDDS()->Lock(NULL, &m_ddsd, DDLOCK_WAIT, NULL);
	}

    if( SUCCEEDED(rval) )
	{
        m_LockCount++;
		return m_LockCount;
	}

	return rval;
}

//////////////////////////////////////////////////////////////////////////////////
// NAME: UnLock
// 
// PURPOSE: UnLocks the surface and prevents access to display memory.            
//
// INPUT: none
//
// RETURNS: m_LockCount - successful
//          < 0         - failed
//
// NOTE:  Should be called after you have finished with a Lock function. This 
// method will properly restore any lost surfaces when called. 
//
//////////////////////////////////////////////////////////////////////////////////
HRESULT CSurface::UnLock(void)
{
	HRESULT rval;
	
    // If the lock ref count is > 1 just decrement it and return success
    if( m_LockCount > 1 )
    {
        m_LockCount--;
        return m_LockCount;
    }

	// Make sure the DX surface is valid
	if(m_pdds == NULL)
		return -1;

	rval = GetDDS()->Unlock(NULL);

	while(rval == DDERR_SURFACELOST) 
    {
		GetDDS()->Restore();
		rval = GetDDS()->Unlock(NULL);
	}

    if( SUCCEEDED(rval) )
	{
        m_LockCount--;
		return m_LockCount;
	}

	return rval;
}



//////////////////////////////////////////////////////////////////////////////////
// NAME: VLine8
// 
// PURPOSE: Draws a vertical line on the surface from Y1 to Y2 at X in colour Col.            
//
// INPUT: Y1  - Y position of the first endpoint
//        Y2  - Y position of the second endpoint
//        X   - X position of the both endpoints
//        Col - color of the line
//
// RETURNS: 0 or > - successful
//          < 0    - failed
//
////////////////////////////////////////////////////////////////////////
HRESULT CSurface::VLine8(int Y1, int Y2, int X, DWORD Col)
{
    int Length,top,bottom;
    BYTE* Pixel;


    Pixel = (BYTE*)m_ddsd.lpSurface;

	// Determine which is top and bottom by the values
    if (Y1 > Y2)
    {
        top = Y2;
        bottom = Y1 + 1;
    }
    else
    {
        top = Y1;
        bottom = Y2 + 1;
    }

    Pixel += top * m_ddsd.lPitch;
    Length = bottom - top;

    // Draw the line
	Pixel += X;
	do
	{
		*Pixel = (BYTE)Col;
		Pixel += m_ddsd.lPitch;
		Length--;
	}while (Length > 0);

    return 0;
}

//////////////////////////////////////////////////////////////////////////////////
// NAME: VLine16
// 
// PURPOSE: Draws a vertical line on the surface from Y1 to Y2 at X in colour Col.            
//
// INPUT: Y1  - Y position of the first endpoint
//        Y2  - Y position of the second endpoint
//        X   - X position of the both endpoints
//        Col - color of the line
//
// RETURNS: 0 or > - successful
//          < 0    - failed
//
////////////////////////////////////////////////////////////////////////
HRESULT CSurface::VLine16(int Y1, int Y2, int X, DWORD Col)
{
    int Length,top,bottom;
    BYTE* Pixel;

    Pixel = (BYTE*)m_ddsd.lpSurface;

	// Determine which is top and bottom by the values
    if (Y1 > Y2)
    {
        top = Y2;
        bottom = Y1 + 1;
    }
    else
    {
        top = Y1;
        bottom = Y2 + 1;
    }


    Pixel += top * m_ddsd.lPitch;
    Length = bottom - top;

    // Draw the line
	Pixel += X << 1;

	do
	{
		*((WORD*)Pixel) = (WORD)Col;
		Pixel += m_ddsd.lPitch;
		Length--;
	}while(Length > 0);

    return 0;
}

//////////////////////////////////////////////////////////////////////////////////
// NAME: VLine24
// 
// PURPOSE: Draws a vertical line on the surface from Y1 to Y2 at X in colour Col.            
//
// INPUT: Y1  - Y position of the first endpoint
//        Y2  - Y position of the second endpoint
//        X   - X position of the both endpoints
//        Col - color of the line
//
// RETURNS: 0 or > - successful
//          < 0    - failed
//
////////////////////////////////////////////////////////////////////////
HRESULT CSurface::VLine24(int Y1, int Y2, int X, DWORD Col)
{
    int Length,top,bottom;
	DWORD dwCol;
    BYTE* Pixel;


    Pixel = (BYTE*)m_ddsd.lpSurface;

	// Determine which is top and bottom by the values
    if (Y1 > Y2)
    {
        top = Y2;
        bottom = Y1 + 1;
    }
    else
    {
        top = Y1;
        bottom = Y2 + 1;
    }


    Pixel += top * m_ddsd.lPitch;
    Length = bottom - top;

    // Draw the line
	Pixel += X + X + X;

	dwCol = Col & 0xFFFF;
	Col = (Col >> 16) & 0xFF;
	do
	{
		*((WORD*)Pixel) = (WORD)dwCol;
		Pixel += 2;
		*Pixel = (BYTE)Col;
		Pixel += m_ddsd.lPitch - 2;
		Length--;
	}while(Length > 0);

    return 0;
}

//////////////////////////////////////////////////////////////////////////////////
// NAME: VLine32
// 
// PURPOSE: Draws a vertical line on the surface from Y1 to Y2 at X in colour Col.            
//
// INPUT: Y1  - Y position of the first endpoint
//        Y2  - Y position of the second endpoint
//        X   - X position of the both endpoints
//        Col - color of the line
//
// RETURNS: 0 or > - successful
//          < 0    - failed
//
////////////////////////////////////////////////////////////////////////
HRESULT CSurface::VLine32(int Y1, int Y2, int X, DWORD Col)
{
    int Length,top,bottom;
//	DWORD dwCol;
    BYTE* Pixel;


    Pixel = (BYTE*)m_ddsd.lpSurface;

	// Determine which is top and bottom by the values
    if (Y1 > Y2)
    {
        top = Y2;
        bottom = Y1 + 1;
    }
    else
    {
        top = Y1;
        bottom = Y2 + 1;
    }


    Pixel += top * m_ddsd.lPitch;
    Length = bottom - top;

    // Draw the line
	Pixel += X << 2;

//	dwCol = Col & 0xFFFF;
//	Col = (Col >> 16) & 0xFF;
	do
	{
//		*((DWORD*)Pixel) = dwCol;
		*((DWORD*)Pixel)=Col;
		Pixel += m_ddsd.lPitch;
		Length--;
	}while(Length > 0);

    return 0;
}

//////////////////////////////////////////////////////////////////////////////////
// NAME: HLine8
// 
// PURPOSE: Draws a horizontal line on the surface from X1 to X2 at Y in colour Col.            
//
// INPUT: X1  - X position of the first endpoint
//        X2  - X position of the second endpoint
//        Y   - Y position of the both endpoints
//        Col - color of the line
//
// RETURNS: 0 or > - successful
//          < 0    - failed
// 
////////////////////////////////////////////////////////////////////////
HRESULT CSurface::HLine8(int X1, int X2, int Y, DWORD Col)
{
    int Length,left,right;
	int i;
    DWORD dwCol;
    BYTE* Pixel;

    Pixel = (BYTE*)m_ddsd.lpSurface;

	// Determine which is left and right by value
    if (X1 > X2)
    {
        left = X2;
        right = X1 + 1;
    }
    else
    {
        left = X1;
        right = X2 + 1;
    }


	// Calculate the length of the line
    Length = right - left;
    Pixel += Y * m_ddsd.lPitch;

	Pixel += left;
	i = Length % 4;
	Length -= i;
	for (i; i > 0; i--)
	{
		*Pixel = (BYTE)Col;
		Pixel++;
	}
	if (Length > 3)
	{
		dwCol = Col | (Col << 8) | (Col << 16) | (Col << 24);
		do
		{
			*((DWORD*)Pixel) = dwCol;
			Pixel += 4;
			Length -= 4;
		}while(Length > 0);
	}

    return 0;
}

//////////////////////////////////////////////////////////////////////////////////
// NAME: HLine16
// 
// PURPOSE: Draws a horizontal line on the surface from X1 to X2 at Y in colour Col.            
//
// INPUT: X1  - X position of the first endpoint
//        X2  - X position of the second endpoint
//        Y   - Y position of the both endpoints
//        Col - color of the line
//
// RETURNS: 0 or > - successful
//          < 0    - failed
// 
////////////////////////////////////////////////////////////////////////
HRESULT CSurface::HLine16(int X1, int X2, int Y, DWORD Col)
{
    int Length,left,right;
	int i;
    DWORD dwCol;
    BYTE* Pixel;


    Pixel = (BYTE*)m_ddsd.lpSurface;

	// Determine which is left and right by value
    if (X1 > X2)
    {
        left = X2;
        right = X1 + 1;
    }
    else
    {
        left = X1;
        right = X2 + 1;
    }


	// Calculate the length of the line
    Length = right - left;
    Pixel += Y * m_ddsd.lPitch;

	Pixel += left << 1;
	i = Length % 2;
	Length -= i;
	for (i; i > 0; i--)
	{
		*((WORD*)Pixel) = (WORD)Col;
		Pixel += 2;
	}
	if (Length > 1)
	{
		dwCol = Col | (Col << 16);
		do
		{
			*((DWORD*)Pixel) = dwCol;
			Pixel += 4;
			Length -= 2;
		}while(Length > 0);
	}

    return 0;
}

//////////////////////////////////////////////////////////////////////////////////
// NAME: HLine24
// 
// PURPOSE: Draws a horizontal line on the surface from X1 to X2 at Y in colour Col.            
//
// INPUT: X1  - X position of the first endpoint
//        X2  - X position of the second endpoint
//        Y   - Y position of the both endpoints
//        Col - color of the line
//
// RETURNS: 0 or > - successful
//          < 0    - failed
// 
////////////////////////////////////////////////////////////////////////
HRESULT CSurface::HLine24(int X1, int X2, int Y, DWORD Col)
{
    int Length,left,right;
    DWORD dwCol;
    BYTE* Pixel;

    Pixel = (BYTE*)m_ddsd.lpSurface;

	// Determine which is left and right by value
    if (X1 > X2)
    {
        left = X2;
        right = X1 + 1;
    }
    else
    {
        left = X1;
        right = X2 + 1;
    }


	// Calculate the length of the line
    Length = right - left;
    Pixel += Y * m_ddsd.lPitch;

	Pixel += left + left + left;

	dwCol = Col & 0xFFFF;
	Col = (Col >> 16) & 0xFF;
	do
	{
		*((WORD*)Pixel) = (WORD)dwCol;
		Pixel += 2;
		*Pixel = (BYTE)Col;
		Pixel++;
		Length--;
	}while(Length > 0);

    return 0;
}

//////////////////////////////////////////////////////////////////////////////////
// NAME: HLine32
// 
// PURPOSE: Draws a horizontal line on the surface from X1 to X2 at Y in colour Col.            
//
// INPUT: X1  - X position of the first endpoint
//        X2  - X position of the second endpoint
//        Y   - Y position of the both endpoints
//        Col - color of the line
//
// RETURNS: 0 or > - successful
//          < 0    - failed
// 
////////////////////////////////////////////////////////////////////////
HRESULT CSurface::HLine32(int X1, int X2, int Y, DWORD Col)
{
    int Length,left,right;
//    DWORD dwCol;
    BYTE* Pixel;

    Pixel = (BYTE*)m_ddsd.lpSurface;

	// Determine which is left and right by value
    if (X1 > X2)
    {
        left = X2;
        right = X1 + 1;
    }
    else
    {
        left = X1;
        right = X2 + 1;
    }

	// Calculate the length of the line
    Length = right - left;
    Pixel += Y * m_ddsd.lPitch;

	Pixel += left << 2;

//	dwCol = Col & 0xFFFF;
//	Col = (Col >> 16) & 0xFF;
	do
	{
//		*((DWORD*)Pixel) = dwCol;
		*((DWORD*)Pixel)=Col;
		Pixel += 4;
		Length--;
	}while(Length > 0);

    return 0;
}


void CSurface::FunctionMapper(void)
{
    DDSURFACEDESC2 ddsd;

	//get a surface description
    ddsd.dwSize = sizeof( ddsd );
    ddsd.dwFlags = DDSD_PIXELFORMAT;

    if (m_pdds->GetSurfaceDesc ( &ddsd ) != DD_OK )
        return;

    switch(ddsd.ddpfPixelFormat.dwRGBBitCount)
    {
    case 8:
        m_fpPutPixel = &CSurface::PutPixel8;
	    //m_fpPutAAPixel = &CSurface::PutAAPixel8;
	    //m_fpGetPixel    = &CSurface::GetPixel8;
	    m_fpVLine      = &CSurface::VLine8;
	    m_fpHLine      = &CSurface::HLine8;
        break;

    case 15:
        m_fpPutPixel   = &CSurface::PutPixel16;
	    //m_fpPutAAPixel = &CSurface::PutAAPixel15;
	    //m_fpGetPixel    = &CSurface::GetPixel16;
	    m_fpVLine      = &CSurface::VLine16;
	    m_fpHLine      = &CSurface::HLine16;
        break;

    case 16:
        m_fpPutPixel   = &CSurface::PutPixel16;
	    //m_fpPutAAPixel = &CSurface::PutAAPixel16;
	    //m_fpGetPixel    = &CSurface::GetPixel16;
	    m_fpVLine      = &CSurface::VLine16;
	    m_fpHLine      = &CSurface::HLine16;
        break;

    case 24:
        m_fpPutPixel   = &CSurface::PutPixel24;
	    //m_fpPutAAPixel = &CSurface::PutAAPixel24;
	    //m_fpGetPixel    = &CSurface::GetPixel24;
	    m_fpVLine      = &CSurface::VLine24;
	    m_fpHLine      = &CSurface::HLine24;
        break;

    case 32:
        m_fpPutPixel   = &CSurface::PutPixel32;
	    //m_fpPutAAPixel = &CSurface::PutAAPixel32;
	    //m_fpGetPixel    = &CSurface::GetPixel32;
	    m_fpVLine      = &CSurface::VLine32;
	    m_fpHLine      = &CSurface::HLine32;
        break;

    default:
        m_fpPutPixel   = NULL;
	    //m_fpPutAAPixel = NULL;
	    //m_fpGetPixel    = NULL;
	    m_fpVLine      = NULL;
	    m_fpHLine      = NULL;
        break;
    }
}

//////////////////////////////////////////////////////////////////////////////////
// NAME: PutPixel8
// 
// PURPOSE: Draws a single pixel to the surface at position X,Y in colour Col.            
//
// INPUT: X   - X location of the pixel
//        Y   - Y location of the pixel
//        Col - color of the pixel
//
// RETURNS: 0 or > - successful
//          < 0    - failed
// 
//////////////////////////////////////////////////////////////////////////////////
HRESULT CSurface::PutPixel8(int X, int Y, DWORD Col)
{
    BYTE* Bitmap;

    // This function is inline, so no function call penalty
    Bitmap = (BYTE*)GetSurfacePointer();

	Bitmap += Y * m_ddsd.lPitch + X;
	*Bitmap = (BYTE)Col;

    return 0;
}

//////////////////////////////////////////////////////////////////////////////////
// NAME: PutPixel16
// 
// PURPOSE: Draws a single pixel to the surface at position X,Y in colour Col.            
//
// INPUT: X   - X location of the pixel
//        Y   - Y location of the pixel
//        Col - color of the pixel
//
// RETURNS: 0 or > - successful
//          < 0    - failed
// 
//////////////////////////////////////////////////////////////////////////////////
HRESULT CSurface::PutPixel16(int X, int Y, DWORD Col)
{
    BYTE* Bitmap;

    // This function is inline, so no function call penalty
    Bitmap = (BYTE*)GetSurfacePointer();

    Bitmap += Y * m_ddsd.lPitch + X * 2;
    *((WORD*)Bitmap) = (WORD)Col;

    return 0;
}

//////////////////////////////////////////////////////////////////////////////////
// NAME: PutPixel24
// 
// PURPOSE: Draws a single pixel to the surface at position X,Y in colour Col.            
//
// INPUT: X   - X location of the pixel
//        Y   - Y location of the pixel
//        Col - color of the pixel
//
// RETURNS: 0 or > - successful
//          < 0    - failed
// 
//////////////////////////////////////////////////////////////////////////////////
HRESULT CSurface::PutPixel24(int X, int Y, DWORD Col)
{
    BYTE* Bitmap;

    // This function is inline, so no function call penalty
    Bitmap = (BYTE*)GetSurfacePointer();
    
    Bitmap += Y * m_ddsd.lPitch + X * 3;
    *((WORD*)Bitmap) = (WORD)(Col & 0xFFFF);
    Bitmap += 2;
    *Bitmap = (BYTE)((Col >> 16) & 0xFF);

    return 0;
}

//////////////////////////////////////////////////////////////////////////////////
// NAME: PutPixel32
// 
// PURPOSE: Draws a single pixel to the surface at position X,Y in colour Col.            
//
// INPUT: X   - X location of the pixel
//        Y   - Y location of the pixel
//        Col - color of the pixel
//
// RETURNS: 0 or > - successful
//          < 0    - failed
// 
///////////////////////////////////////////////////////////////////////////////////
HRESULT CSurface::PutPixel32(int X, int Y, DWORD Col)
{
    BYTE* Bitmap;

    // This function is inline, so no function call penalty
    Bitmap = (BYTE*)GetSurfacePointer();

    Bitmap += Y * m_ddsd.lPitch + X * 4;
	*((DWORD*)Bitmap) = Col; 

    return 0;
}

// this can be optimized : we are using 4 locks here !
HRESULT CSurface::Rectangle(int X, int Y, int W, int H, COLORREF Colref)
{
	Line(X,Y, X+W, Y, Colref);
	Line(X,Y, X, Y+H, Colref);

	Line(X+W, Y, X+W, Y+H, Colref);
	Line(X, Y+H, X+W, Y+H, Colref);

	return 0;
}



HRESULT CSurface::Line(int X1, int Y1, int X2, int Y2, COLORREF Colref)
{
	DWORD Col = ConvertGDIColor(Colref);

	double xStep, yStep, X, Y;
	int xLength, yLength, xCount, yCount;
    HRESULT rval;

    // Lock the surface once instead of multiple times
    rval = Lock();

    // If we couldn't lock the surface return failure code
    if( FAILED(rval) )
        return -1;

    // If the line is horizontal or vertical use the fast version.
	if (X1 == X2)
	{
		rval = VLine(Y1,Y2,X1,Col);
        rval = UnLock();
		return rval;
	}
	else if (Y1 == Y2)
	{
		rval = HLine(X1,X2,Y1,Col);
        rval = UnLock();
		return rval;
	}

	xLength = abs(X2 - X1);
	yLength = abs(Y2 - Y1);

	if(xLength == 0) VLine(Y1, Y2, X1, Col);
	else if(yLength == 0) HLine(X1, X2, Y1, Col);

	else if(xLength > yLength)
	{
		if(X1 > X2)
		{
			Swap(&X1, &X2);
			Swap(&Y1, &Y2);
		}

		yStep = (double)(Y2 - Y1) / (double)(X2 - X1);
		Y = Y1;

		for(xCount = X1; xCount <= X2; xCount++)
		{
			PutPixel(xCount, (int)Y, Col);
			Y += yStep;
		}
	}
	else
	{
		if(Y1 > Y2)
		{
			Swap(&X1, &X2);
			Swap(&Y1, &Y2);
		}

		xStep = (double)(X2 - X1) / (double)(Y2 - Y1);
		X = X1;

		for(yCount = Y1; yCount <= Y2; yCount++)
		{
			PutPixel((int)X, yCount, Col);
			X += xStep;
		}
	}

    // Unlock the surface
    rval = UnLock();

    // If Unlock failed return failure code
    if( FAILED(rval) )
        return -2;

    return 0;
}