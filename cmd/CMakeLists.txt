SET(folder_package_depend )
SET(folder_link_libs -lstdc++fs)

SET(folder_lib_depend core )

SET(current_dir_lists ${CMAKE_CURRENT_LIST_FILE})

INCLUDE(${CMAKE_HOME_DIRECTORY}/cmake/BuildSrcFolderGenericLists.txt NO_POLICY_SCOPE)
