add_subdirectory(containers)
add_subdirectory(widgets)

add_library(emui-lib SHARED
	emui.c
	event.c
	style.c
	print.c
	text.c
	focus.c
	tile.c
	dbg.c
)

target_link_libraries(emui-lib containers widgets ${CURSES_LIBRARIES})

set_target_properties(emui-lib PROPERTIES
	OUTPUT_NAME "emui"
	SOVERSION ${APP_VERSION_MAJOR}.${APP_VERSION_MINOR}
	PUBLIC_HEADER "${CMAKE_SOURCE_DIR}/include/emui.h"
)

install(TARGETS emui-lib
	LIBRARY DESTINATION lib
	PUBLIC_HEADER DESTINATION include
)

install(FILES
	${CMAKE_SOURCE_DIR}/include/tile.h
	${CMAKE_SOURCE_DIR}/include/tiles.h
	${CMAKE_SOURCE_DIR}/include/focus.h
	${CMAKE_SOURCE_DIR}/include/print.h
	${CMAKE_SOURCE_DIR}/include/style.h
	${CMAKE_SOURCE_DIR}/include/text.h
	DESTINATION include/emui
)

install(FILES
	${CMAKE_SOURCE_DIR}/include/widgets/label.h
	${CMAKE_SOURCE_DIR}/include/widgets/lineedit.h
	${CMAKE_SOURCE_DIR}/include/widgets/misc.h
	${CMAKE_SOURCE_DIR}/include/widgets/textview.h
	DESTINATION include/emui/widgets
)

add_executable(emui
	test.c
)

target_link_libraries(emui emui-lib ${EMCRK_LIBRARIES} ${EMDAS_LIBRARIES})

# vim: tabstop=4
