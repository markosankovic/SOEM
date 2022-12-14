set(SOURCES somanet.c mongoose.c)
add_executable(somanet ${SOURCES})
target_link_libraries(somanet soem)
install(TARGETS somanet DESTINATION bin)
