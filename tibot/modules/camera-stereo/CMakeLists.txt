cmake_minimum_required (VERSION 2.6)
project (o2cl-tibot-camera)

find_package( OpenCV REQUIRED )

add_executable (Camera src/camera.cpp)
target_link_libraries (Camera tiobj)
target_link_libraries( Camera ${OpenCV_LIBS} )

install(TARGETS Camera DESTINATION bin)
