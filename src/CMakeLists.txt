add_subdirectory(tiles)

add_library(emui-lib SHARED
	emui.c
	event.c
	style.c
	print.c
)

target_link_libraries(emui-lib
	tiles
)

set_target_properties(emui-lib PROPERTIES
	OUTPUT_NAME "emui"
	SOVERSION ${EMUI_VERSION_MAJOR}.${EMUI_VERSION_MINOR}
	PUBLIC_HEADER "${CMAKE_SOURCE_DIR}/include/emui.h"
)

install(TARGETS emui-lib
	LIBRARY DESTINATION lib
	PUBLIC_HEADER DESTINATION include
)

install(FILES
	${CMAKE_SOURCE_DIR}/include/emui/iset.h
	${CMAKE_SOURCE_DIR}/include/emui/buf.h
	${CMAKE_SOURCE_DIR}/include/emui/dh.h
	${CMAKE_SOURCE_DIR}/include/emui/errors.h
	DESTINATION include/emui
)

add_executable(emui
	test.c
)

target_link_libraries(emui emui-lib ${EMELF_LIBRARIES} ${EMAWP_LIBRARIES} ${CURSES_LIBRARIES})

install(TARGETS emui RUNTIME DESTINATION bin)

# vim: tabstop=4
