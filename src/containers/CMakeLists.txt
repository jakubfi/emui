add_library(containers
	screen.c
	dummy_cont.c
	window.c
	tabs.c
	splitter.c
	justifier.c
)

set_target_properties(containers
	PROPERTIES POSITION_INDEPENDENT_CODE 1
)

# vim: tabstop=4
