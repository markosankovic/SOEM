set(SOURCES multimedia_timer.cpp)
add_executable(multimedia_timer ${SOURCES})
target_link_libraries(multimedia_timer soem winmm.lib)
install(TARGETS multimedia_timer DESTINATION bin)
