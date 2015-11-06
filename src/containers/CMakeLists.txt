add_library(containers
	screen.c
	dummy_cont.c
	frame.c
	tabs.c
	splitter.c
	justifier.c
	grid.c
)

set_target_properties(containers
	PROPERTIES POSITION_INDEPENDENT_CODE 1
)

# vim: tabstop=4
