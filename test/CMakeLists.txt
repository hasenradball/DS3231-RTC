add_executable(test_DS3231
  test_DS3231.cpp
)

target_include_directories(test_DS3231 PRIVATE
  ../src
)

target_link_libraries(test_DS3231
  GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(test_DS3231)