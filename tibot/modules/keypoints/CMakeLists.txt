cmake_minimum_required (VERSION 2.6)
project (o2cl-tibot-keypoints)

find_package( OpenCV REQUIRED )

add_executable (Keypoints src/keypoints.cpp)
target_link_libraries (Keypoints tiobj)
target_link_libraries( Keypoints ${OpenCV_LIBS} )

install(TARGETS Keypoints DESTINATION bin)
