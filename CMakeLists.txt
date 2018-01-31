# CMakeLists files in this project can
# refer to the root source directory of the project as ${SOES_SOURCE_DIR} and
# to the root binary directory of the project as ${SOES_BINARY_DIR}.
cmake_minimum_required (VERSION 2.8.4)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")
project (SOES)

set (SOES_VERSION_MAJOR 2)
set (SOES_VERSION_MINOR 1)
set (SOES_VERSION_PATCH 3)

# Generate version numbers
configure_file (
  ${SOES_SOURCE_DIR}/version.h.in
  ${SOES_SOURCE_DIR}/soes/version.h
  )

# Include paths
include_directories(
  ${SOES_SOURCE_DIR}
  ${SOES_SOURCE_DIR}/soes
  )

# Platform flags and sources
include(${CMAKE_SYSTEM_NAME} OPTIONAL)

# Source paths
add_subdirectory (soes)
add_subdirectory (${SOES_DEMO})


message (STATUS "Building for ${CMAKE_SYSTEM_NAME}")

include (InstallRequiredSystemLibraries)
set (CPACK_RESOURCE_FILE_LICENSE "${SOES_SOURCE_DIR}/LICENSE")
set (CPACK_PACKAGE_VERSION_MAJOR "${SOES_VERSION_MAJOR}")
set (CPACK_PACKAGE_VERSION_MINOR "${SOES_VERSION_MINOR}")
set (CPACK_PACKAGE_VERSION_PATCH "${SOES_VERSION_MINOR}")
set (CPACK_PACKAGE_CONTACT info.ethercat@rt-labs.com)
include (CPack)
