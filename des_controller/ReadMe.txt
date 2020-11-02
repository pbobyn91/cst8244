Title: des_controller

Author: @came0351@algonquinlive.com, @pbobyn			


Contribution: The controller was done as a group effort. Multiple different state machines were 
created, trying everyone of them to determine the best. A Solid team effort was provided for this project
since EXIT gracefully was also an issue we had for a while.

Example : 
	START -> IDLE 
	START -> IDLE -> GRL | GLL
	
	START -> GRL(IDLE) -> RS|LS This method was chosen since it matches the requirements for GRL and GLL to be the idles.
			 GLL(IDLE) -> RS|LS
Status:  The project works as expected and meets all specification and requirements assigned
		 No missing requirements or runtime errors.

Known Issues: N/A

Expected Grade: A+