cmake_minimum_required(VERSION 3.0.0)
project(infiniteExplorer VERSION 0.1.0)


set(CMAKE_BUILD_TYPE Debug)

add_executable(infiniteExplorer)

# build libInfinite first
set(INFINITE_EXPORT_OODLE ON)
set(INFINITE_COPY_OODLE_DLL ON)
set(INFINITE_OODLE_DLL oo2core_8_win64.dll)
add_subdirectory(libInfinite/)

# all of this copying around is needed with linoodle because linoodle expects the dll to be in the cwd
if(${INFINITE_COPY_OODLE_DLL})
	configure_file(libInfinite/${INFINITE_OODLE_DLL} ${INFINITE_OODLE_DLL} COPYONLY)
endif()


# only use pkgconfig when not cross compiling
if(NOT CUSTOM_GTKMM)
	
	find_package(PkgConfig)
	
	pkg_check_modules(GTKMM gtkmm-3.0)
	
	if(NOT DEFINED GTKMM_INCLUDE_DIRS)
		message(STATUS "Could not find pkgconfig, trying to find gtkmm directly")
		find_package(gtkmm-3.0)
		#find_path(GTKMM_INCLUDE_DIRS gtkmm.h)
		#message(STATUS "GTKmm include at ${GTKMM_INCLUDE_DIRS}")
	else()
		
		
		
		include_directories(${GTKMM_INCLUDE_DIRS})
		#target_link_directories(infiniteExplorer ${GTKMM_LIBRARY_DIRS})
		target_include_directories(infiniteExplorer PRIVATE ${GTKMM_INCLUDE_DIRS})
		target_link_libraries(infiniteExplorer ${GTKMM_LIBRARIES})
		message(STATUS "using system gtkmm from pkgconfig")
	endif()
endif()



set_property(TARGET infiniteExplorer PROPERTY CXX_STANDARD 17)

set(sources PRIVATE main.cpp MainWindow.cpp FileList.cpp ModuleManager.cpp PropertiesDialog.cpp LogViewer.cpp LogManager.cpp ManagedLogger.cpp)

target_sources(infiniteExplorer PRIVATE ${sources})

target_include_directories(infiniteExplorer PUBLIC .)



# link libInfinite
target_link_libraries(infiniteExplorer Infinite)

# copy resource files
include(res/CMakeLists.txt)
