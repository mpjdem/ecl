#ifndef _ECL_DXMANAGER_INCLUDED_
#define _ECL_DXMANAGER_INCLUDED_

#include <ddraw.h>
#include <vector>

#pragma comment(lib, "ddraw.lib")
#pragma comment(lib, "dxguid.lib")

using namespace std;


//--------------------------------------------------------------------
//  CLASS:			ecl_dxmanager
//	DESCRIPTION:	Initializes and manages DirectX functionality
//--------------------------------------------------------------------
class ecl_dxmanager
{
public:
													ecl_dxmanager();
												  ~ ecl_dxmanager();

	static	bool									init();

	// DDraw devices
	static	HRESULT									get_ddraw_devices();
	static  GUID								  * get_monitor_guid(int mon_n);
	static	bool								    allocate_monitor(int mon_n);
	static	int										get_allocated_monitor_count(){return g_allocated_monitors.size();}
	static  const WCHAR							  * get_monitor_name(int mon_n);
	inline	static  int								get_monitor_count(){return g_dwDeviceCount;};
	static  void									get_monitor_rect(int mon_n, LPRECT r);

	// DInput devices
	// (none)


protected:

			struct DDRAW_DEVICE_STRUCT
			{
				GUID  guid;
				LPWSTR  strDescription;
				LPWSTR  strDriverName;
				DWORD dwModeCount;
				SIZE  aModeSize[256];
			};

	static	bool									g_b_initialised;
	static	vector<int>								g_allocated_monitors;

	static	DDRAW_DEVICE_STRUCT						g_aDevices[16];
	static	HMONITOR								g_monitorHandles[16];
	static	int										g_dwDeviceCount;

	static	BOOL WINAPI								DDEnumCallbackEx(GUID* pGUID, LPWSTR strDriverDescription, LPWSTR strDriverName, LPVOID pContext, HMONITOR hm);
	static	HRESULT	WINAPI							EnumModesCallback(LPDDSURFACEDESC pddsd,  LPVOID pContext);
};
#endif	//_ECL_DXMANAGER_INCLUDED_
