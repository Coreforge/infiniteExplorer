add_executable(infiniteExplorerCLI)

target_link_libraries(infiniteExplorerCLI assimp)
set_property(TARGET infiniteExplorerCLI PROPERTY CXX_STANDARD 20)

# CLI source files

target_sources(infiniteExplorerCLI PRIVATE ${CMAKE_CURRENT_LIST_DIR}/main.cpp)

# the exporter code from the 3D folder has to be added separately, since it's compiled into the main executable
target_sources(infiniteExplorerCLI PRIVATE 3D/ExporterBase.cpp)
target_sources(infiniteExplorerCLI PRIVATE 3D/AssImpExporter.cpp)

if(WIN32)
	target_sources(infiniteExplorerCLI PRIVATE res/Icon.rc)
endif()

target_include_directories(infiniteExplorerCLI PUBLIC .)

target_link_libraries(infiniteExplorerCLI Infinite)
target_link_libraries(infiniteExplorerCLI argparse)