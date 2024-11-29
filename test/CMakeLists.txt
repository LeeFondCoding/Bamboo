cmake_minimum_required(VERSION 3.10)


find_package(GTest REQUIRED)

#add_executable(testTimeStamp test_TimeStamp.cc ../net/base/TimeStamp.cc)
add_executable(testTimeStamp net/base/test_time_stamp.cc ${BASE_FILES})
target_link_libraries(testTimeStamp ${GTEST_LIBRARIES})

add_executable(testCurrentThread net/base/test_current_thread.cc ../net/base/CurrentThread.cc)

target_link_libraries(testTimeStamp ${GTEST_LIBRARIES})

add_executable(test_append_file net/base/test_file_util.cc ../net/base/FileUtil.cc)
target_link_libraries(test_append_file ${GTEST_LIBRARIES})

add_executable(test_log_file net/base/test_log_file.cc ../net/base/FileUtil.cc ../net/base/LogFile.cc)
target_link_libraries(test_log_file ${GTEST_LIBRARIES})

add_executable(test_fixed_buf net/base/test_fixed_buffer.cc ../net/base/LogStream.cc)
target_link_libraries(test_fixed_buf ${GTEST_LIBRARIES})

add_executable(test_logstream net/base/test_logstream.cc ../net/base/LogStream.cc)
target_link_libraries(test_logstream ${GTEST_LIBRARIES})

add_executable(testlogger net/base/test_logging.cc ${BASE_FILES})
target_link_libraries(testlogger ${GTEST_LIBRARIES})

add_executable(test_skip_list db/test_skip_list.cc)
target_link_libraries(test_skip_list ${GTEST_LIBRARIES})