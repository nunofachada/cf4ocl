# Available scripts
set(SCRIPTS ccl_plot_events.py)

# Where to install scripts
foreach(SCRIPT ${SCRIPTS})
	install(PROGRAMS ${CMAKE_CURRENT_SOURCE_DIR}/${SCRIPT}
		DESTINATION ${CMAKE_INSTALL_BINDIR})
endforeach()

