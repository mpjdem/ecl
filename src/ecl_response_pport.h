#ifndef _ECL_RESPONSE_PPORT_INCLUDED_
#define _ECL_RESPONSE_PPORT_INCLUDED_

#include "ecl.h"
#include "ecl_response.h"

#define _T(x)      L ## x 
typedef short (_stdcall *inpfuncPtr)(short portaddr);

using namespace std;


//--------------------------------------------------------------------
//  CLASS:			ecl_response_pport
//	DESCRIPTION:	Parallel port response child class
//--------------------------------------------------------------------
class ecl_response_pport : public ecl_response
{
public:

															ecl_response_pport(long port_address);
	virtual												  ~ ecl_response_pport();

	// Base class implementations
			bool											init();
			bool											chresp_pressed(long button_n);
			long											chresp_get_axis(long axis_n);
			
			
    // Own implementations
    // (none)


protected:

	// Base class implementations
	// (none)
			
			
    // Own implementations
			long											m_port_address;
			HINSTANCE										m_hLib;
			inpfuncPtr										m_inp32fp;
			
};
//

#endif	//_ECL_RESPONSE_PPORT_INCLUDED_