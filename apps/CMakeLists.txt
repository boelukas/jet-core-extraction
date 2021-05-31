add_executable(jet_cmd jet_cmd.cpp)
target_link_libraries(jet_cmd PUBLIC jet stdc++fs netcdf)
target_include_directories(jet_cmd PUBLIC "${PROJECT_SOURCE_DIR}/include" "${PROJECT_SOURCE_DIR}/extern/nanoflann/include")
target_link_directories(jet_cmd PUBLIC "${SOURCE_DIR}/include")