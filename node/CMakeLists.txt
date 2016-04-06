cmake_minimum_required (VERSION 2.6)
project ("o2cl-node")
set(CMAKE_BUILD_TYPE RELEASE)


add_executable (class "src/class.cpp" )
target_link_libraries (class "tiobj" "tisys" )
INSTALL(TARGETS class DESTINATION bin)
add_executable (is "src/is.cpp" )
target_link_libraries (is "tiobj" "tisys" )
INSTALL(TARGETS is DESTINATION bin)
add_executable (isnot "src/isnot.cpp" )
target_link_libraries (isnot "tiobj" "tisys" )
INSTALL(TARGETS isnot DESTINATION bin)
