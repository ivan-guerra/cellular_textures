set(TESTNAME common_test)

add_executable(${TESTNAME} placeholder.cpp)

target_link_libraries(${TESTNAME} PRIVATE gtest_main)

gtest_discover_tests(${TESTNAME} WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/tests)

set_target_properties(${TESTNAME} PROPERTIES FOLDER tests)
