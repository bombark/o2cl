cmake_minimum_required (VERSION 2.6)
project (o2cl-tibot-kinect)

add_executable (Kinect src/kinect.c)
target_link_libraries (Kinect m)
target_link_libraries (Kinect pthread)
target_link_libraries (Kinect freenect)

install(TARGETS Kinect DESTINATION bin)
