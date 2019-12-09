#include "stdafx.h"

#include <windows.h>

#include "ecl.h"
#include "ecl_response_keyboard.h"

using namespace std;


//--------------------------------------------------------------------
//	FUNCTION:		ecl_response_keyboard::ecl_response_keyboard()
//	DESCRIPTION:	Constructor
//--------------------------------------------------------------------
ecl_response_keyboard::ecl_response_keyboard()
{
	m_b_initialised = false;

	m_n_buttons = 0;
	m_n_axes = 0;
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_response_keyboard::~ecl_response_keyboard()
//	DESCRIPTION:	Deconstructor
//--------------------------------------------------------------------
ecl_response_keyboard::~ecl_response_keyboard()
{
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_response_keyboard::init()
//	DESCRIPTION:	Initialisation routine
//--------------------------------------------------------------------
bool ecl_response_keyboard::init()
{
	if (m_b_initialised)
	{
		ecl_error("ECL: ecl_response_keyboard::init() - Object has already been initialised");
		return false;
	}
	
	// Default keys
	add_button("SPACEBAR",VK_SPACE);
	add_button("ESCAPE",VK_ESCAPE);
	
	// Done
	m_b_initialised = true;
	return true;
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_response_keyboard::chresp_pressed()
//	DESCRIPTION:	Check button press
//--------------------------------------------------------------------
bool ecl_response_keyboard::chresp_pressed(long button_n)
{	
	long key;

	key = m_button_devns[button_n-1];
	
	return	( (GetAsyncKeyState(key) & 0x8000) != 0);
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_response_keyboard::chresp_get_axis()
//	DESCRIPTION:	Check button press
//--------------------------------------------------------------------
long ecl_response_keyboard::chresp_get_axis(long axis_n)
{	
	ecl_error("ECL: ecl_response_keyboard_chresp_get_axis() - A keyboard has no axes");
	return 0;
}
//
