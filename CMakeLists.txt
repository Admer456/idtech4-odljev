cmake_minimum_required (VERSION 3.2.0)
project (fhDOOM)

OPTION(ID_ALLOW_TOOLS "Compile Tools" ON)
OPTION(ID_ALLOW_MAYA "Compile Maya Tools" ON)
OPTION(ID_UNICODE "Use unicode version of WIN32 API" OFF)

include(setup.cmake)

include_directories(neo/renderer)
include_directories(neo/glew)
add_subdirectory (neo/idlib)
IF("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  add_subdirectory (neo/curl)
  add_subdirectory (neo/TypeInfo)
  if(ID_ALLOW_MAYA)
    add_subdirectory (neo/maya2md5)
  endif()
ENDIF()  
add_subdirectory (neo/game)
add_subdirectory (neo/d3xp)
add_subdirectory (neo)


