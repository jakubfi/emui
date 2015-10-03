add_subdirectory(containers)
add_subdirectory(widgets)
add_subdirectory(windows)

add_library(tiles
	tile.c
)

set_target_properties(tiles
	PROPERTIES POSITION_INDEPENDENT_CODE 1
)

target_link_libraries(tiles
	containers
	widgets
	windows
)

# vim: tabstop=4
