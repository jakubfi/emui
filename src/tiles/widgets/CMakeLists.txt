add_library(widgets
	framecounter.c
	fpscounter.c
	lineedit.c
)

set_target_properties(widgets
	PROPERTIES POSITION_INDEPENDENT_CODE 1
)

# vim: tabstop=4
