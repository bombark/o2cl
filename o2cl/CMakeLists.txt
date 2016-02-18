cmake_minimum_required (VERSION 2.6)
project (o2cl)
add_executable (o2cl src/main.cpp)
target_link_libraries (o2cl tiobj tisys tiarg)

install(TARGETS o2cl DESTINATION bin)
