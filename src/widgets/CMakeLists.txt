add_library(widgets
	framecounter.c
	fpscounter.c
	lineedit.c
	label.c
	textview.c
	line.c
)

set_target_properties(widgets
	PROPERTIES POSITION_INDEPENDENT_CODE 1
)

# vim: tabstop=4
