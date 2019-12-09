#ifndef _ECL_REGION_CIRCLE_INCLUDED_
#define _ECL_REGION_CIRCLE_INCLUDED_

#include "ecl_region.h"

using namespace std;



//--------------------------------------------------------------------
//  CLASS:			ecl_region_circle
//	DESCRIPTION:	Circular screen region child class
//--------------------------------------------------------------------
class ecl_region_circle : public ecl_region
{
public:

															ecl_region_circle();
															ecl_region_circle(long center_x, long center_y, long radius);
	virtual												  ~ ecl_region_circle();
	

	// Base class implementations
			bool											in_region(long x, long y);

	// Own implementations
			bool											set(long center_x, long center_y, long radius);
			

						
protected:
	// Base class implementations
			bool											draw_func(unsigned long preload, long center_x, long center_y, ecl_monitor * mon=NULL, ecl_LUTc c=-1);


	// Own implementations
			long											m_radius;
};
//

#endif	//_ECL_REGION_CIRCLE_INCLUDED_