#----------------------------------------------------------------
# Generated CMake target import file.
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "SimpleLog::Log" for configuration ""
set_property(TARGET SimpleLog::Log APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(SimpleLog::Log PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_NOCONFIG "CXX"
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/lib/libLog.a"
  )

list(APPEND _cmake_import_check_targets SimpleLog::Log )
list(APPEND _cmake_import_check_files_for_SimpleLog::Log "${_IMPORT_PREFIX}/lib/libLog.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
