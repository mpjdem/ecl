#include "stdafx.h"

#include <windows.h>

#include "ecl.h"
#include "ecl_response_pport.h"

using namespace std;


//--------------------------------------------------------------------
//	FUNCTION:		ecl_response_pport::ecl_response_pport()
//	DESCRIPTION:	Constructor
//--------------------------------------------------------------------
ecl_response_pport::ecl_response_pport(long port_address)
{
	m_port_address = port_address;
	m_b_initialised = false;

	m_n_buttons = 0;
	m_n_axes = 0;

	m_hLib = NULL;
	inpfuncPtr m_inp32fp=NULL;
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_response_pport::~ecl_response_pport()
//	DESCRIPTION:	Deconstructor
//--------------------------------------------------------------------
ecl_response_pport::~ecl_response_pport()
{
	if (m_hLib != NULL)
	{
		FreeLibrary(m_hLib);
		m_hLib = NULL;
	}

	inpfuncPtr m_inp32fp=NULL;
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_response_pport::init()
//	DESCRIPTION:	Initialization routine
//--------------------------------------------------------------------
bool ecl_response_pport::init()
{
	if (m_b_initialised)
	{
		ecl_error("ECL: ecl_response_pport::init() - Object has already been initialised");
		return false;
	}
	
	m_hLib = LoadLibrary(_T("inpout32.dll"));

	if (m_hLib == NULL)
	{
		ecl_error("ECL: ecl_response_pport::init() - Cannot load inpout32.dll");
		return false;
	}

	m_inp32fp = (inpfuncPtr) GetProcAddress(m_hLib, "Inp32");
	if (m_inp32fp == NULL) 
	{
		ecl_error("ECL: ecl_response_pport::init() - Cannot load inpout32.dll");
		return false;
	}

	// Default keys
	add_button("LEFT",1);
	add_button("RIGHT",2);
	
	// Done
	m_b_initialised = true;

	return true;
}
//

//--------------------------------------------------------------------
//	FUNCTION:		ecl_response_pport::chresp_pressed()
//	DESCRIPTION:	Check button press
//--------------------------------------------------------------------
bool ecl_response_pport::chresp_pressed(long button_n)
{	
	if (!m_b_initialised || m_inp32fp == NULL)
	{
		ecl_error("ECL: ecl_response_pport::chresp_pressed() - Parallel port has not been initialized");
		return false;
	}
	
	long key;

	key = m_button_devns[button_n-1];

	short mask = (1 << (key-1));
	short v = (short) m_inp32fp(m_port_address);

	return ( (v & mask) != 0 );
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_response_pport::chresp_get_axis()
//	DESCRIPTION:	Check axis
//--------------------------------------------------------------------
long ecl_response_pport::chresp_get_axis(long axis_n)
{	
	ecl_error("ECL: ecl_response_pport::chresp_get_axis() - A parallel port has no axes");
	return 0;
}
//

