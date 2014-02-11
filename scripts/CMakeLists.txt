# Available scripts
set(SCRIPTS plot_events.py)

# Where to install scripts
foreach(SCRIPT ${SCRIPTS})
	install(PROGRAMS ${CMAKE_CURRENT_SOURCE_DIR}/${SCRIPT}
		DESTINATION bin
		RENAME ${EXECS_PREFIX}${SCRIPT})
endforeach()

