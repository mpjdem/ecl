#include "stdafx.h"

#pragma warning(disable: 4786)

#include <windows.h>

#if !defined(COMPILE_MULTIMON_STUBS) && (WINVER < 0x0500)
	#define COMPILE_MULTIMON_STUBS
	#include "multimon.h"
#endif

#include "ecl.h"
#include "ecl_dxmanager.h"


// Static member variables
bool									ecl_dxmanager::g_b_initialised;
vector<int>								ecl_dxmanager::g_allocated_monitors;
ecl_dxmanager::DDRAW_DEVICE_STRUCT		ecl_dxmanager::g_aDevices[16];
HMONITOR								ecl_dxmanager::g_monitorHandles[16];
int										ecl_dxmanager::g_dwDeviceCount;



//--------------------------------------------------------------------
//	FUNCTION:		ecl_dxmanager::ecl_dxmanager()
//	DESCRIPTION:	Constructor
//--------------------------------------------------------------------
ecl_dxmanager::ecl_dxmanager()
{
	g_dwDeviceCount = 0;
	g_b_initialised = false;
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_dxmanager::~ecl_dxmanager()
//	DESCRIPTION:	Deconstructor
//--------------------------------------------------------------------
ecl_dxmanager::~ecl_dxmanager()
{
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_dxmanager::init()
//	DESCRIPTION:	Deconstructor
//--------------------------------------------------------------------
bool ecl_dxmanager::init()
{
	if (!g_b_initialised)
	{
		get_ddraw_devices();
		g_b_initialised = true;
	}

	return true;
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_dxmanager::get_ddraw_devices()
//	DESCRIPTION:	Enumerate the GUIDs that are present
//--------------------------------------------------------------------
HRESULT ecl_dxmanager::get_ddraw_devices()
{
    return DirectDrawEnumerateEx(DDEnumCallbackEx,NULL,DDENUM_ATTACHEDSECONDARYDEVICES);
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_dxmanager::DDEnumCallbackEx()
//	DESCRIPTION:	Callback to this function for every GUID enumerated
//--------------------------------------------------------------------
BOOL WINAPI ecl_dxmanager::DDEnumCallbackEx(GUID* pGUID, LPWSTR strDriverDescription, LPWSTR strDriverName, LPVOID pContext, HMONITOR hm)
{
    HRESULT hr;
    LPDIRECTDRAW pDD = NULL;

    // Create a device out of the enumerated GUID
	hr = DirectDrawCreateEx(pGUID, (VOID**)&pDD, IID_IDirectDraw7, NULL);

    if(SUCCEEDED(hr))
    {
        // Add it to the global storage structure
		if(pGUID)
        {
            g_aDevices[g_dwDeviceCount].guid = *pGUID;
        }
        else
        {
            ZeroMemory(&g_aDevices[g_dwDeviceCount].guid, sizeof(GUID) );
        }

        // Copy the description of the driver into the structure
        lstrcpyn(g_aDevices[g_dwDeviceCount].strDescription,
                  strDriverDescription, 256 );
        lstrcpyn( g_aDevices[g_dwDeviceCount].strDriverName,
                  strDriverName, 64 );
		g_monitorHandles[g_dwDeviceCount]=hm;

        // Retrieve the modes this device can support
        g_aDevices[g_dwDeviceCount].dwModeCount = 0;
        hr = pDD->EnumDisplayModes(0, NULL, NULL, EnumModesCallback);

        // Increase the counter for the number of devices found
        g_dwDeviceCount++;

        // Release this device
        if(pDD)
		{
			pDD->Release();
			pDD=NULL;
		}
    }

    // Continue looking for more devices
    return TRUE;
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_dxmanager::EnumModesCallback()
//	DESCRIPTION:	Callback to this function for mode enumerated
//--------------------------------------------------------------------
HRESULT WINAPI ecl_dxmanager::EnumModesCallback(LPDDSURFACEDESC pddsd,LPVOID pContext)
{
    DWORD i;
    DWORD dwModeSizeX;
    DWORD dwModeSizeY;
    DWORD dwModeCount;

    // For each mode, look through all previously found modes
    // to see if this mode has already been added to the list
    dwModeCount = g_aDevices[ g_dwDeviceCount ].dwModeCount;

    for( i = 0; i < dwModeCount; i ++ )
    {
        dwModeSizeX = g_aDevices[ g_dwDeviceCount ].aModeSize[i].cx;
        dwModeSizeY = g_aDevices[ g_dwDeviceCount ].aModeSize[i].cy;

        if ( ( dwModeSizeX == pddsd->dwWidth ) &&
             ( dwModeSizeY == pddsd->dwHeight ) )
        {
            // If this mode has been added, then stop looking
            break;
        }
    }

    // If this mode was not in g_aDevices[g_dwDeviceCount].aModeSize[]
    // then added it.
    if( i == g_aDevices[ g_dwDeviceCount ].dwModeCount )
    {
        g_aDevices[ g_dwDeviceCount ].aModeSize[i].cx = pddsd->dwWidth;
        g_aDevices[ g_dwDeviceCount ].aModeSize[i].cy = pddsd->dwHeight;

        // Increase the number of modes found for this device
        g_aDevices[ g_dwDeviceCount ].dwModeCount++;
    }

    return TRUE;
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_dxmanager::get_monitor_guid()
//	DESCRIPTION:	Return the GUID of a given monitor
//--------------------------------------------------------------------
GUID * ecl_dxmanager::get_monitor_guid(int mon_n)
{
	if (mon_n<0||mon_n>=g_dwDeviceCount)
	{return NULL;}
	else
	{return &(g_aDevices[mon_n].guid);}
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_dxmanager::allocate_monitor()
//	DESCRIPTION:	Store the GUID pointers that have been used
//--------------------------------------------------------------------
bool ecl_dxmanager::allocate_monitor(int mon_n)
{
    vector<int>::iterator it;
	for (it=g_allocated_monitors.begin(); it < g_allocated_monitors.end(); it++)
	{
		if (*it == mon_n)
		{
			ecl_error("ECL: ecl_dxmanager::allocate_monitor() - Monitor has already been allocated");
			return false;
		}
	}

	g_allocated_monitors.push_back(mon_n);
	return true;
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_dxmanager::get_monitor_name()
//	DESCRIPTION:	Return the name of a given monitor
//--------------------------------------------------------------------
const WCHAR * ecl_dxmanager::get_monitor_name(int mon_n)
{
	if (mon_n<0||mon_n>=g_dwDeviceCount)
	{return L"No such monitor";}
	else
	{return g_aDevices[mon_n].strDescription;}
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_dxmanager::get_monitor_rect()
//	DESCRIPTION:	Return the RECT area of a given monitor
//--------------------------------------------------------------------
void ecl_dxmanager::get_monitor_rect(int mon_n, LPRECT r)
{
	if (mon_n < 0 || mon_n > g_dwDeviceCount)
	{return;}

	MONITORINFO mi;
    RECT        rc;

	mi.cbSize = sizeof( mi );
	::GetMonitorInfo( g_monitorHandles[mon_n], &mi );
	rc = mi.rcMonitor;

	::SetRect( r, rc.left, rc.top, rc.right, rc.bottom );
}
//
