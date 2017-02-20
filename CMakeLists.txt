cmake_minimum_required(VERSION 2.8.11)
project(evo-test)

set(C_FLAGS "-g -Wall -fPIC" CACHE STRING "")
set(CXX_FLAGS "${C_FLAGS} -std=c++11 -fno-exceptions" CACHE STRING "")

set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} ${C_FLAGS})
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} ${CXX_FLAGS})

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Widgets REQUIRED)
include_directories(${Qt5Core_INCLUDE_DIRS})
include_directories(${Qt5Gui_INCLUDE_DIRS})
include_directories(${Qt5Widgets_INCLUDE_DIRS})

set(LIBLA "libla" CACHE INTERNAL "")
include_directories(${LIBLA})

add_subdirectory(libgenn-qt)
include_directories(${LIBGENN_QT})
set(LIBS ${LIBS} genn-qt)

include_directories(source)
set(SOURCE source/main.cpp)

add_executable(${PROJECT_NAME} ${SOURCE})

set(LIBS ${LIBS} pthread)
set(LIBS ${LIBS} Qt5::Widgets)

target_link_libraries(${PROJECT_NAME} ${LIBS})
