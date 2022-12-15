set(SOURCES playground.c)
add_executable(playground ${SOURCES})
target_link_libraries(playground soem)
install(TARGETS playground DESTINATION bin)
