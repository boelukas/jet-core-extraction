file(GLOB SRC_FILE_LIST CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/src/*.hpp" "${PROJECT_SOURCE_DIR}/src/*.cpp")

add_executable(jet_cmd ${SRC_FILE_LIST})
target_link_libraries(jet_cmd PUBLIC stdc++fs netcdf)
target_include_directories(jet_cmd PUBLIC "${PROJECT_SOURCE_DIR}/include" "${PROJECT_SOURCE_DIR}/extern/nanoflann/include")
target_link_directories(jet_cmd PUBLIC "${SOURCE_DIR}/include")
