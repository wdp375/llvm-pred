cmake_minimum_required(VERSION 2.8)
project(llvm-pred)

set(CMAKE_MODULE_PATH 
   ${CMAKE_MODULE_PATH}
   ${CMAKE_SOURCE_DIR}/cmake
   )

if(NOT CMAKE_BUILD_TYPE)
   set(CMAKE_BUILD_TYPE "Release")
endif()

find_package(LLVM)

option(DYNAMIC_LINK "Link Dynamic LLVM Libraries" ${LLVM_DYNAMIC_LIBRARY_FOUND})
option(WITH_DEBUG "Enable Extra Debug Output" OFF)
option(WITH_MORE_DEBUG "Enable More Extra Debug Output" Off)
set(PROMOTE_FREQ "select" CACHE STRINGS "Select how to fix promote errors in performance predication")
set_property(CACHE PROMOTE_FREQ PROPERTY STRINGS "select" "path_prob")
option(UNIT_TEST "Enable Build Unit Test" Off)
option(TC_USE_SCEV "LoopTripcount use SCEV" Off)
option(DELETE_STORE "Delete the Store instructions" Off)
option(DELETE_RETURN "Delete the Return instructions" Off)
option(USE_CACHE "Use Cache to Speedup Cacluate" Off)
option(USE_PROMOTE "Use Promote Algorithm to promote view port" OFF)
option(USE_DOUBLE_ARRAY "Use the array with double type to do the instrumentation" ON)


find_package(PkgConfig)
pkg_check_modules(LLVM_PROF llvm-prof REQUIRED)

if(NOT WITH_DEBUG)
   set(NO_DEBUG On)
endif()
set(ANNOY_DEBUG ${WITH_MORE_DEBUG})
configure_file(config.h.in config.h)
configure_file(llvm-pred.pc.in llvm-pred.pc @ONLY)
install(FILES ${CMAKE_BINARY_DIR}/llvm-pred.pc DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/pkgconfig)
message(STATUS "Add ${CMAKE_INSTALL_PREFIX}/lib/pkgconfig to $PKG_CONFIG_PATH "
"environment variable to let pkg-config find llvm-pred.pc")

add_subdirectory(src)
if(UNIT_TEST)
add_subdirectory(unit)
endif()
