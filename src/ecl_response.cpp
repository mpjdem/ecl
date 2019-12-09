#include "stdafx.h"
#include "ecl.h"
#include "ecl_response.h"

using namespace std;


//--------------------------------------------------------------------
//	FUNCTION:		ecl_response::ecl_response()
//	DESCRIPTION:	Constructor
//--------------------------------------------------------------------
ecl_response::ecl_response()
{
	m_b_initialised = false;

	m_n_buttons = 0;
	m_n_axes = 0;
			
	m_button_devns.clear();
	m_axis_devns.clear();
	m_button_names.clear();
	m_axis_names.clear();	
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_response::~ecl_response()
//	DESCRIPTION:	Deconstructor
//--------------------------------------------------------------------
ecl_response::~ecl_response()
{
	m_button_devns.clear();
	m_axis_devns.clear();
	m_button_names.clear();
	m_axis_names.clear();	
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_response::add_button()
//	DESCRIPTION:	Deconstructor
//--------------------------------------------------------------------
bool ecl_response::add_button(string name, long devn)
{
	vector<string>::iterator it;
	for (it=m_button_names.begin(); it < m_button_names.end(); it++)
	{
		if (*it == name)
		{
			ecl_error("ECL: ecl_response::add_button() - Attempted redefinition of button");
			return false;
		}
	}
	
	m_button_devns.push_back(devn);
	m_button_names.push_back(name);

	m_n_buttons++;

	return true;
}



//--------------------------------------------------------------------
//	FUNCTION:		ecl_response::add_axis()
//	DESCRIPTION:	Deconstructor
//--------------------------------------------------------------------
bool ecl_response::add_axis(string name, long devn)
{
	vector<string>::iterator it;
	for (it=m_axis_names.begin(); it < m_axis_names.end(); it++)
	{
		if (*it == name)
		{
			ecl_error("ECL: ecl_response::add_axis() - Attempted redefinition of axis");
			return false;
		}
	}
	
	m_axis_devns.push_back(devn);
	m_axis_names.push_back(name);
	
	m_n_axes++;

	return true;
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_response::get_name()
//	DESCRIPTION:	Retrieve name
//--------------------------------------------------------------------
string ecl_response::get_name(long n)
{
	if (!m_b_initialised)
	{
		ecl_error("ECL: ecl_response::get_name() - ecl_response device has not been initialised");
		return "";
	}

	if (!m_n_buttons)
	{
		ecl_error("ECL: ecl_response::get_name() - no buttons defined");
		return "";
	}

	if (n >= m_n_buttons)
	{
		ecl_error("ECL: ecl_response::get_name() - invalid button number");
		return "";
	}

	return m_button_names[n];
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_response::is_pressed()
//	DESCRIPTION:	Checks whether a device button has been pressed
//--------------------------------------------------------------------
bool ecl_response::is_pressed(string button_name)
{
	if (!m_b_initialised)
	{
		ecl_error("ECL: ecl_response::is_pressed() - ecl_response device has not been initialised");
		return false;
	}

	if (!m_n_buttons)
	{
		ecl_error("ECL: ecl_response::is_pressed() - no buttons defined");
		return false;
	}

	vector<string>::iterator it;
	
	long button_n= 0;
	for (it=m_button_names.begin(); it < m_button_names.end(); it++)
	{
		button_n++;

		if (*it == button_name)
		{
			break;
		}
	}
	
	return is_pressed(button_n);
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_response::is_pressed()
//	DESCRIPTION:	Checks whether a device button has been pressed
//--------------------------------------------------------------------
bool ecl_response::is_pressed(long button_n)
{
	if (!m_b_initialised)
	{
		ecl_error("ECL: ecl_response::is_pressed() - ecl_response device has not been initialised");
		return false;
	}

	if (!m_n_buttons)
	{
		ecl_error("ECL: ecl_response::is_pressed() - no buttons defined");
		return false;
	}
	
	if (!button_n)
	{
		ecl_error("ECL: ecl_response::is_pressed() - Button name was not found");
		return false;
	}
	else if (button_n > m_n_buttons)
	{
		ecl_error("ECL: ecl_response::is_pressed() - Button number exceeds maximum");
		return false;
	}
	else
		{return chresp_pressed(button_n);}
}


//--------------------------------------------------------------------
//	FUNCTION:		ecl_response::wait_until__pressed()
//	DESCRIPTION:	Waits until the device button has been pressed
//--------------------------------------------------------------------
void ecl_response::wait_until_pressed(string button_name)
{
	while(!is_pressed(button_name))
		{}
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_response::wait_until__released()
//	DESCRIPTION:	Waits until the button has been released for t ms
//--------------------------------------------------------------------
void ecl_response::wait_until_released(string button_name, long t)
{
	bool done = false;
	bool tmpdone = false;
	long tmark = 0;

	while(!done)
	{
		if(!is_pressed(button_name))
		{		
			if (!tmpdone)
			{
				tmark = ecl_get_time();
				tmpdone = true;
			}
		}
		else
		{
			tmark = 0;
			tmpdone = false;
		}

		if (tmpdone && ecl_get_time()-tmark>t)
		{
			done = true;
		}			
	}
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_response::wait_until_pressed_then_released()
//	DESCRIPTION:	Combines the two previous functions
//--------------------------------------------------------------------
void ecl_response::wait_until_pressed_then_released(string button_name, long t)
{
	wait_until_pressed(button_name);
	wait_until_released(button_name,t);
}
//


//--------------------------------------------------------------------
//	FUNCTION:		ecl_response::get_axis()
//	DESCRIPTION:	Returns an axis value
//--------------------------------------------------------------------
long ecl_response::get_axis(string axis_name)
{
	if (!m_b_initialised)
	{
		ecl_error("ECL: ecl_response::get_axis() - ecl_response device has not been initialised");
		return false;
	}

	if (!m_n_axes)
	{
		ecl_error("ECL: ecl_response::get_axis() - No axes defined");
		return false;
	}

	vector<string>::iterator it;
	
	long axis_n= 0;
	for (it=m_axis_names.begin(); it < m_axis_names.end(); it++)
	{
		axis_n++;

		if (*it == axis_name)
		{
			break;
		}
	}
	
	
	if (!axis_n)
	{
		ecl_error("ECL: ecl_response::get_axis() - Axis name was not found");
		return false;
	}
	else if (axis_n > m_n_axes)
	{
		ecl_error("ECL: ecl_response::get_axis() - Axis number exceeds maximum");
		return false;
	}
	else
		{return chresp_get_axis(axis_n);}
}
//
