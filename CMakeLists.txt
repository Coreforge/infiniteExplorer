cmake_minimum_required(VERSION 3.0.0)
project(infiniteExplorer VERSION 0.1.0)

add_executable(infiniteExplorer)



find_package(PkgConfig)

pkg_check_modules(GTKMM gtkmm-3.0)

include_directories(${GTKMM_INCLUDE_DIRS})
#target_link_directories(infiniteExplorer ${GTKMM_LIBRARY_DIRS})
target_include_directories(infiniteExplorer PRIVATE ${GTKMM_INCLUDE_DIRS})


set_property(TARGET infiniteExplorer PROPERTY CXX_STANDARD 17)

set(sources PRIVATE main.cpp MainWindow.cpp FileList.cpp)

target_sources(infiniteExplorer PRIVATE ${sources})

target_include_directories(infiniteExplorer PUBLIC .)

target_link_libraries(infiniteExplorer ${GTKMM_LIBRARIES})

# copy resource files
include(res/CMakeLists.txt)