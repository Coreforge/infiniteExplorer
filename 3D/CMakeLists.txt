add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/yttrium)
include(${CMAKE_CURRENT_LIST_DIR}/interface/CMakeLists.txt)

target_sources(infiniteExplorer PRIVATE ${CMAKE_CURRENT_LIST_DIR}/ExporterBase.cpp)
target_sources(infiniteExplorer PRIVATE ${CMAKE_CURRENT_LIST_DIR}/AssImpExporter.cpp)