# Note that headers are optional, and do not affect add_library, but they will not
# show up in IDEs unless they are listed in add_library.

# Optionally glob, but only for CMake 3.12 or later:
file(GLOB HEADER_LIST CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/src/*.hpp" "${PROJECT_SOURCE_DIR}/src/*.cpp")
message(STATUS "Header list: ${PROJECT_SOURCE_DIR}")

#set(HEADER_LIST "${ModernCMakeExample_SOURCE_DIR}/include/modern/lib.hpp")

# Make an automatic library - will be static or dynamic based on user setting
add_executable(jet_cmd ${HEADER_LIST})
target_link_libraries(jet_cmd PUBLIC stdc++fs netcdf)
target_include_directories(jet_cmd PUBLIC "${PROJECT_SOURCE_DIR}/include" "${PROJECT_SOURCE_DIR}/extern/nanoflann/include")
target_link_directories(jet_cmd PUBLIC "${SOURCE_DIR}/include")
