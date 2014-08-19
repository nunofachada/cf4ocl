# List of aux. build programs
set(AUX_BUILD_PROGS get_data_path)

# Add a target for each aux. build program
foreach(AUX_BUILD_PROG ${AUX_BUILD_PROGS})
	add_executable(${AUX_BUILD_PROG} ${AUX_BUILD_PROG}.c)
	target_link_libraries(${AUX_BUILD_PROG} 
		${GLIB_LIBRARIES} ${GLIB_LDFLAGS})
endforeach()


