#include "stdafx.h"		

#pragma warning(disable: 4786)

#include <cstdio>
#include <vector>
#include <string>
#include <time.h>	
#include <windows.h>

#include "ecl.h"

using namespace std;


//--------------------------------------------------------------------
//	GLOBAL VARIABLES
//--------------------------------------------------------------------
LONGLONG			g_windows_timer_start_time=0;
LARGE_INTEGER		g_highPerformanceFreq={0,0};
vector<string>		ecl_error_buf;
//


//--------------------------------------------------------------------
//	FUNCTION:		DllMain
//	DESCRIPTION:	DLL generation entry point
//--------------------------------------------------------------------
/*BOOL APIENTRY DllMain( HANDLE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
    switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
				if (!::QueryPerformanceFrequency(&g_highPerformanceFreq))
					{return FALSE;}
				ecl_reset_timer();

			break;
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
    }
    return TRUE;
}
//
*/


//--------------------------------------------------------------------
//	FUNCTION:		ecl_init
//	DESCRIPTION:	General initialization routine
//--------------------------------------------------------------------
bool ecl_init()
{
	if (!::QueryPerformanceFrequency(&g_highPerformanceFreq))
		{return false;}

	ecl_reset_timer();
	long x = time(NULL);
	for (int i=0;i<100;i++)
	{
		srand(x);
		x=rand();
	}

	ecl_error_buf.reserve(ECL_ERROR_BUFFER_SIZE);

	return true;
}
//


//--------------------------------------------------------------------
//  FUNCTION:		ecl_cleanup
//	DESCRIPTION:	General clean-up routine
//--------------------------------------------------------------------
bool ecl_cleanup()
{
	ecl_error_buf.clear();
	return true;
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_reset_timer
//	DESCRIPTION:	Reset CPU-based Windows timer
//--------------------------------------------------------------------
long ecl_reset_timer()
{
	static LARGE_INTEGER t;
	QueryPerformanceCounter(&t);
	g_windows_timer_start_time = (ECL_SAMPLE_FREQ*t.QuadPart/g_highPerformanceFreq.QuadPart);
	return 0;
}
//


//---------------------------------------------------------------------
//	FUNCTION:		ecl_get_time
//	DESCRIPTION:	Retrieve time since reset (in clock ticks)
//---------------------------------------------------------------------
long ecl_get_time()
{
	static LARGE_INTEGER t;
	QueryPerformanceCounter(&t);
	LONGLONG ll = t.QuadPart;

    return  (long)(((ECL_SAMPLE_FREQ*t.QuadPart)/g_highPerformanceFreq.QuadPart) - g_windows_timer_start_time);
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_wait
//	DESCRIPTION:	Wait for a given time (in clock ticks)
//--------------------------------------------------------------------
void ecl_wait(long t)
{
	long end_time = ecl_get_time() + t;
	while (ecl_get_time() < end_time)
	{
	}
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_make_LUTcolor
//	DESCRIPTION:	Create a LUT color
//--------------------------------------------------------------------
ecl_LUTc ecl_make_LUTcolor(int r, int g, int b)
{
	return RGB(r,g,b);
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_error
//	DESCRIPTION:	Add an error message to the error buffer
//--------------------------------------------------------------------
bool ecl_error(string err_msg)
{
	if (ecl_error_buf.size() < ECL_ERROR_BUFFER_SIZE)
	{
		ecl_error_buf.push_back(err_msg);
		return true;
	}
	else
	{
		ecl_die("ECL: ecl_error() - Error buffer is full");
		return false;
	}
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_dump_error_buffer
//	DESCRIPTION:	Write the error buffer to a file
//--------------------------------------------------------------------
bool ecl_dump_error_buf()
{
	vector<string>::iterator it;
	FILE * error_dump = fopen(ECL_ERROR_BUFFER_NAME,"w+");

	if (error_dump != NULL)
	{
		for (it=ecl_error_buf.begin(); it < ecl_error_buf.end(); it++)
			{fprintf(error_dump,"%s\n",(*it).c_str());}
		
		fclose(error_dump);
	
		return true;
	}
	else
		{return false;}
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_die
//	DESCRIPTION:	Throw an exception
//--------------------------------------------------------------------
bool ecl_die(string err_msg)
{
	ecl_dump_error_buf();
	throw(err_msg);
	return false;
}
//


