cmake_minimum_required(VERSION 3.10)
project(monitor VERSION 1.0)

find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIRS})

include_directories(include)
file(GLOB SOURCES "src/*.cpp")

add_executable(${PROJECT_NAME} ${SOURCES} include/ncurses_display.h)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)
target_link_libraries("${PROJECT_NAME}" ${CURSES_LIBRARIES})
# TODO: Run -Werror in CI.
target_compile_options("${PROJECT_NAME}" PRIVATE -Wall -Wextra)
