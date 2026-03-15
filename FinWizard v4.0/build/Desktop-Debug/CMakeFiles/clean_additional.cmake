# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles/Raz_ZP_qt_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/Raz_ZP_qt_autogen.dir/ParseCache.txt"
  "QXlsx_build/CMakeFiles/QXlsx_autogen.dir/AutogenUsed.txt"
  "QXlsx_build/CMakeFiles/QXlsx_autogen.dir/ParseCache.txt"
  "QXlsx_build/QXlsx_autogen"
  "Raz_ZP_qt_autogen"
  )
endif()
