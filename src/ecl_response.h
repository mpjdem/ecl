#ifndef _ECL_RESPONSE_INCLUDED_
#define _ECL_RESPONSE_INCLUDED_

#include <vector>
#include <string>

using namespace std;



//--------------------------------------------------------------------
//  CLASS:			ecl_response
//	DESCRIPTION:	Response device base class
//--------------------------------------------------------------------
class ecl_response
{
public:

															ecl_response();
	virtual												  ~ ecl_response();

	// Base class implemented
			bool											add_button(string name, long devn);
			bool											add_axis(string name, long devn);

    inline  bool                                            is_initialised(){return m_b_initialised;}

			string											get_name(long n);

			bool											is_pressed(string button_name);
			bool											is_pressed(long button_n);

			void											wait_until_pressed(string button_name);
			void											wait_until_released(string button_name, long t);
			void											wait_until_pressed_then_released(string button_name, long t);

			long											get_axis(string axis_name);
			
			
    // Child class implemented
    virtual	bool											init()=0;


protected:

    // Base class implemented
			bool											m_b_initialised;

			long											m_n_buttons;
			long											m_n_axes;
			
			vector<long>									m_button_devns;
			vector<long>									m_axis_devns;	
			vector<string>									m_button_names;
			vector<string>									m_axis_names;	


    // Child class implemented
	virtual	bool											chresp_pressed(long button_n)=0;
	virtual	long											chresp_get_axis(long axis_n)=0;
};
//

#endif	//_ECL_RESPONSE_INCLUDED_