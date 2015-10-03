add_library(windows
	window.c
)

set_target_properties(windows
	PROPERTIES POSITION_INDEPENDENT_CODE 1
)

# vim: tabstop=4
