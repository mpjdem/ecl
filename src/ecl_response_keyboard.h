#ifndef _ECL_RESPONSE_KEYBOARD_INCLUDED_
#define _ECL_RESPONSE_KEYBOARD_INCLUDED_

#include "ecl_response.h"

using namespace std;



//--------------------------------------------------------------------
//  CLASS:			ecl_response_keyboard
//	DESCRIPTION:	Keyboard response child class
//--------------------------------------------------------------------
class ecl_response_keyboard : public ecl_response
{
public:

															ecl_response_keyboard();
	virtual												  ~ ecl_response_keyboard();

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
	// (none)
};
//

#endif	//_ECL_RESPONSE_KEYBOARD_INCLUDED_