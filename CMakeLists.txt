cmake_minimum_required( VERSION 3.10 )
project(SDLTest)

add_subdirectory(SDL2)

set(SDL2_INCLUDE_DIRS SDL2/include)
set(SDL2_LIBRARIES SDL2;SDL2main)
link_directories(SDL2/build)


include_directories(${SDL2_INCLUDE_DIRS})
IF(WIN32)
add_executable(SDLTest WIN32 main.cpp)
ELSE()
add_executable(SDLTest main.cpp)
ENDIF()
target_link_libraries(SDLTest ${SDL2_LIBRARIES})