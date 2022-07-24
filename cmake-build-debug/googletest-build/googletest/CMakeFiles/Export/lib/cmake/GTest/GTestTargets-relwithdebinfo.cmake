#----------------------------------------------------------------
# Generated CMake target import file for configuration "RelWithDebInfo".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "GTest::gtest" for configuration "RelWithDebInfo"
set_property(TARGET GTest::gtest APPEND PROPERTY IMPORTED_CONFIGURATIONS RELWITHDEBINFO)
set_target_properties(GTest::gtest PROPERTIES
  IMPORTED_LOCATION_RELWITHDEBINFO "${_IMPORT_PREFIX}/lib/libgtest.1.12.1.dylib"
  IMPORTED_SONAME_RELWITHDEBINFO "@rpath/libgtest.1.12.1.dylib"
  )

list(APPEND _IMPORT_CHECK_TARGETS GTest::gtest )
list(APPEND _IMPORT_CHECK_FILES_FOR_GTest::gtest "${_IMPORT_PREFIX}/lib/libgtest.1.12.1.dylib" )

# Import target "GTest::gtest_main" for configuration "RelWithDebInfo"
set_property(TARGET GTest::gtest_main APPEND PROPERTY IMPORTED_CONFIGURATIONS RELWITHDEBINFO)
set_target_properties(GTest::gtest_main PROPERTIES
  IMPORTED_LOCATION_RELWITHDEBINFO "${_IMPORT_PREFIX}/lib/libgtest_main.1.12.1.dylib"
  IMPORTED_SONAME_RELWITHDEBINFO "@rpath/libgtest_main.1.12.1.dylib"
  )

list(APPEND _IMPORT_CHECK_TARGETS GTest::gtest_main )
list(APPEND _IMPORT_CHECK_FILES_FOR_GTest::gtest_main "${_IMPORT_PREFIX}/lib/libgtest_main.1.12.1.dylib" )

# Import target "GTest::gmock" for configuration "RelWithDebInfo"
set_property(TARGET GTest::gmock APPEND PROPERTY IMPORTED_CONFIGURATIONS RELWITHDEBINFO)
set_target_properties(GTest::gmock PROPERTIES
  IMPORTED_LOCATION_RELWITHDEBINFO "${_IMPORT_PREFIX}/lib/libgmock.1.12.1.dylib"
  IMPORTED_SONAME_RELWITHDEBINFO "@rpath/libgmock.1.12.1.dylib"
  )

list(APPEND _IMPORT_CHECK_TARGETS GTest::gmock )
list(APPEND _IMPORT_CHECK_FILES_FOR_GTest::gmock "${_IMPORT_PREFIX}/lib/libgmock.1.12.1.dylib" )

# Import target "GTest::gmock_main" for configuration "RelWithDebInfo"
set_property(TARGET GTest::gmock_main APPEND PROPERTY IMPORTED_CONFIGURATIONS RELWITHDEBINFO)
set_target_properties(GTest::gmock_main PROPERTIES
  IMPORTED_LOCATION_RELWITHDEBINFO "${_IMPORT_PREFIX}/lib/libgmock_main.1.12.1.dylib"
  IMPORTED_SONAME_RELWITHDEBINFO "@rpath/libgmock_main.1.12.1.dylib"
  )

list(APPEND _IMPORT_CHECK_TARGETS GTest::gmock_main )
list(APPEND _IMPORT_CHECK_FILES_FOR_GTest::gmock_main "${_IMPORT_PREFIX}/lib/libgmock_main.1.12.1.dylib" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
