SECTIONS
{
	apptestsuite__param_arg : {
	   __start_apptestsuite__param_arg = .;
		KEEP (*(apptestsuite__param_arg))
	  __stop_apptestsuite__param_arg = .;
	}
}
INSERT AFTER .rodata
