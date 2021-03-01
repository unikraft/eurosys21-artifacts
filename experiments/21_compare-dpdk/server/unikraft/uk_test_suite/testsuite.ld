SECTIONS
{
	__start_testsuite_fns = LOADADDR(testsuite);
	testsuite : {
		KEEP (*(testsuite_fn))
	}
	__end_testsuite_fns = LOADADDR(testsuite)+SIZEOF(testsuite);

	__start_testsuite_arg_fns = LOADADDR(testsuite_args);
	testsuite_args : {
		KEEP (*(testsuite_fn_args))
	}
	__end_testsuite_arg_fns = LOADADDR(testsuite_args)+SIZEOF(testsuite_args);

}
INSERT AFTER .rodata
