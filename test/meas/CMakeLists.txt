set(SOURCES meas.c)
add_executable(meas ${SOURCES})
target_link_libraries(meas soem)
install(TARGETS meas DESTINATION bin)
