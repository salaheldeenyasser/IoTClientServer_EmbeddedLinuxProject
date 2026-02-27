# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles/IoTServer_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/IoTServer_autogen.dir/ParseCache.txt"
  "IoTServer_autogen"
  )
endif()
