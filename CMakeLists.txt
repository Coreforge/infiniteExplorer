cmake_minimum_required(VERSION 3.6.0)
project(infiniteExplorer VERSION 0.1.0)

cmake_policy(SET CMP0079 NEW)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_STANDARD 20)

add_executable(infiniteExplorer)

# build libInfinite first
set(INFINITE_EXPORT_OODLE ON)
set(INFINITE_COPY_OODLE_DLL ON)
set(INFINITE_OODLE_DLL oo2core_8_win64.dll)
set(FUSE ON)

option(YTTRIUM_3D_VIEWER "Build the 3D viewer" ON)
option(BUILD_CLI_VERSION "Build CLI" ON)
option(SANITIZE_ADDRESS "use ASAN" OFF)




############################################################################
#
#	For some reason, setting I can't add anything to PKG_CONFIG_PATH
#	from within CMake, so it has to be done outside as an 
#	environment variable
#
#	Because the .pc files don't support static linking, this path is still needed though
if(WIN32)
# where pkg-config can find the .pc files for winFSP installed by chocolatey
set(WINFSP_DIR "/c/Program Files (x86)/WinFSP/lib")
endif()

if(SANITIZE_ADDRESS)
	add_compile_options(-fsanitize=address)
	add_link_options(-fsanitize=address)
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-omit-frame-pointer -fsanitize=address")
	set (CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fno-omit-frame-pointer -fsanitize=address")
endif()

if(NOT DEFINED LIBINFINITE)
	add_subdirectory(libInfinite/)
else()
	add_library(Infinite UNKNOWN IMPORTED)
	message(STATUS "Using libInfinite from ${LIBINFINITE}")
	set_target_properties(Infinite PROPERTIES IMPORTED_LOCATION ${LIBINFINITE})
endif()

# all of this copying around is needed with linoodle because linoodle expects the dll to be in the cwd
if(${INFINITE_COPY_OODLE_DLL})
	configure_file(libInfinite/${INFINITE_OODLE_DLL} ${INFINITE_OODLE_DLL} COPYONLY)
endif()


# 3D stuff
if(YTTRIUM_3D_VIEWER)
	include(3D/CMakeLists.txt)
	target_include_directories(infiniteExplorer PRIVATE 3D/yttrium/)
	target_link_libraries(infiniteExplorer yttrium)
endif()

#include(net/CMakeLists.txt)

# FUSE
if(FUSE)
	find_package(PkgConfig)
	#if(WIN32)
	#	message(STATUS "adding WinFSP to PKG_CONFIG_PATH")
	#	set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH}:${WINFSP_DIR}")
	#endif()
	pkg_check_modules(FUSE IMPORTED_TARGET fuse3)
	
	if(NOT DEFINED FUSE_INCLUDE_DIRS)
		message(STATUS "Could not find libfuse with pkgconfig")
		find_package(FUSE)
		if(NOT FUSE_FOUND)
			if(WIN32)
				message(STATUS "If you're trying to use winFSP, add the path to the lib folder to the PKG_CONFIG_PATH environment variable")
			endif()
			message(WARNING "Could not find libFuse. Disabling FUSE support")
			set(FUSE OFF)
		else()
			target_include_directories(infiniteExplorer PRIVATE ${FUSE_INCLUDE_DIRS})
			target_link_libraries(infiniteExplorer ${FUSE_LIBRARIES})
		endif()
	else()
		string(REPLACE ";" " " FUSE_INCLUDE_DIRS "${FUSE_INCLUDE_DIRS}")
		string(REPLACE ";" " " FUSE_LDFLAGS "${FUSE_LDFLAGS}")
		message(STATUS "${FUSE_LDFLAGS}")
		target_include_directories(infiniteExplorer PRIVATE "${FUSE_INCLUDE_DIRS}")
		target_link_libraries(infiniteExplorer "${FUSE_LDFLAGS}")
	endif()
endif()

#GTKmm
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

# Assimp
find_package(assimp REQUIRED)
target_link_libraries(infiniteExplorer assimp)

#find_package(jsoncpp REQUIRED)
#target_link_libraries(infiniteExplorer jsoncpp)
#find_package(PNG REQUIRED)
#target_link_libraries(infiniteExplorer ${PNG_LIBRARIES})

set_property(TARGET infiniteExplorer PROPERTY CXX_STANDARD 20)

set(sources PRIVATE main.cpp
					MainWindow.cpp
					FileList.cpp
					ModuleDisplayManager.cpp
					PropertiesDialog.cpp
					LogViewer.cpp
					LogManager.cpp
					ManagedLogger.cpp
					FileViewerManager.cpp
					InfiniteFileViewer.cpp
					ClosableTab.cpp
					DataTableViewer.cpp
					ContentTableViewer.cpp
					StringTableViewer.cpp
					TextureViewer.cpp
					StringUtils.cpp
					MeshViewer.cpp
					BspViewer.cpp
					VartViewer.cpp
					VectorViewer.cpp
					PmdfViewer.cpp
					materialDialog.cpp)
					
target_sources(infiniteExplorer PRIVATE BitmapViewer.cpp)
		
if(FUSE)
	set(sources ${sources} FuseDialog.cpp FuseProvider.cpp)
	target_compile_definitions(infiniteExplorer PRIVATE USE_FUSE)
endif()
					
					
if(WIN32)
	target_sources(infiniteExplorer PRIVATE ${sources}  res/Icon.rc)
else()
	target_sources(infiniteExplorer PRIVATE ${sources})
endif()

target_include_directories(infiniteExplorer PUBLIC .)

if(BUILD_CLI_VERSION)
	add_subdirectory(argparse/)
	include(cli/CMakeLists.txt)
endif()



# link libInfinite
target_link_libraries(infiniteExplorer Infinite)

# copy resource files
include(res/CMakeLists.txt)

configure_file(libInfinite/3rd_party/detex/LICENSE detex_LICENSE COPYONLY)
