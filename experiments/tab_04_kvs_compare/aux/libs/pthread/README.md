pthread-embedded for Unikraft
=============================

This is the port of pthread-embedded for Unikraft as external library. You will
need newlib to make it work. When adding the library in the dependency list,
pthread-embedded should stay before newlib, e.g.:

 `...:$(UK_LIBS)/pthread-embedded:$(UK_LIBS)/newlib:...`

For running the unit tests, enable the 'Build unit tests' option in the
configuration menu and call the `pte_test_main()` function in your main
application.

Please refer to the `README.md` as well as the documentation in the `doc/`
subdirectory of the main unikraft repository.
