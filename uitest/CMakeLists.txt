add_executable(uitest
	main.c
	statusbar.c
	help.c
	goto.c
	uregs.c
	sregs.c
	dasm.c
	memory.c
	debugger.c
)

target_link_libraries(uitest emui-lib ${EMCRK_LIBRARIES} ${EMDAS_LIBRARIES})

# vim: tabstop=4
