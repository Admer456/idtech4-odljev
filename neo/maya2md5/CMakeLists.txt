
if (NOT "${MAYA_HOME}" STREQUAL "")
  message("MAYA_HOME from options: ${MAYA_HOME}")
elseif(DEFINED ENV{MAYA_HOME})
  set(MAYA_HOME $ENV{MAYA_HOME})
  message("MAYA_HOME from environment: $ENV{MAYA_HOME}")
else()
  set(PROGRAMFILES "PROGRAMFILES(X86)")
  set(MAYA_HOME "$ENV{${PROGRAMFILES}}/Autodesk/Maya2011/")  
  message("MAYA_HOME assumed: ${MAYA_HOME}")
endif()

IF(IS_DIRECTORY ${MAYA_HOME})

  SET(SOURCES
    exporter.h
    main.cpp
    maya_main.cpp
    maya_main.h
    maya.h
    ModelExport.h
    ModelExport.cpp
    ../framework/CVarSystem.cpp  
    ../framework/CVarSystem.h
    ../framework/CmdSystem.cpp
    ../framework/CmdSystem.h
    ../framework/FileSystem.cpp
    ../framework/FileSystem.h
    ../framework/File.cpp
    ../framework/File.h
    ../framework/Licensee.h
    ../framework/Unzip.cpp
    ../framework/Unzip.h
  )

  include_directories("${MAYA_HOME}/include")
  add_definitions(-DID_TYPEINFO -D__DOOM_DLL__ -DID_ENABLE_CURL=0)
  add_executable(maya2md5 ${SOURCES})
  target_link_libraries(maya2md5
    idlib
    "${MAYA_HOME}/lib/Foundation.lib"
    "${MAYA_HOME}/lib/OpenMaya.lib"
    "${MAYA_HOME}/lib/OpenMayaAnim.lib"
    )

  SET_PROPERTY(TARGET maya2md5 PROPERTY FOLDER exes)
ELSE()
  message(WARNING "Maya2011 SDK not found. maya2md5 tool is excluded from the build")  
ENDIF()