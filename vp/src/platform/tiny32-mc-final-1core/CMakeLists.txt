file(GLOB_RECURSE HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

add_executable(tiny32-mc-final-1core
        mc_main.cpp
        ${HEADERS})

target_link_libraries(tiny32-mc-final-1core rv32 platform-common gdb-mc
	${Boost_LIBRARIES} ${SystemC_LIBRARIES} pthread)

INSTALL(TARGETS tiny32-mc-final-1core RUNTIME DESTINATION bin)
