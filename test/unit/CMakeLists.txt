################################
# GTest
################################
find_package(GTest)
if(NOT ${GTEST_FOUND})
    include(ExternalProject)
    ExternalProject_Add(googletest
      URL https://github.com/google/googletest/archive/release-1.8.0.zip
      PREFIX ${CMAKE_CURRENT_BINARY_DIR}/gtest
      INSTALL_COMMAND ""
    )
    ExternalProject_Get_Property(googletest source_dir binary_dir)

    set(GTEST_INCLUDE_DIRS ${source_dir}/googletest/include)
    set(GTEST_LIBRARIES_PATH ${binary_dir}/googlemock/gtest/${CMAKE_FIND_LIBRARY_PREFIXES}gtest.a)
    set(GTEST_MAIN_LIBRARIES_PATH ${binary_dir}/googlemock/gtest/${CMAKE_FIND_LIBRARY_PREFIXES}gtest_main.a)
    set(GTEST_LIBRARIES gtest)
    set(GTEST_MAIN_LIBRARIES gtest_main)
    set(GTEST_BOTH_LIBRARIES ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES})
    add_library(${GTEST_LIBRARIES} UNKNOWN IMPORTED)
    add_library(${GTEST_MAIN_LIBRARIES} UNKNOWN IMPORTED)
    set_property(TARGET ${GTEST_LIBRARIES} PROPERTY IMPORTED_LOCATION ${GTEST_LIBRARIES_PATH})
    set_property(TARGET ${GTEST_MAIN_LIBRARIES} PROPERTY IMPORTED_LOCATION ${GTEST_MAIN_LIBRARIES_PATH})
    add_dependencies(${GTEST_LIBRARIES} googletest)
    add_dependencies(${GTEST_MAIN_LIBRARIES} googletest)
endif()


################################
# Unit Tests
################################
enable_testing()

set(tests
    proc
    base64
    charset
    big_int
    big_float
    speed
    random
    md
    mac
    crc
    bloom_filter
    ## 其它的测试程序完成后可以加到这里，比如： http_client http_server img_sdl kcp_client mysql mongodb redis rwlock nats_client
    )

include_directories(${GTEST_INCLUDE_DIRS})
message(${GTEST_INCLUDE_DIR})

foreach(test ${tests})
    add_executable(${test} ${test}.cpp)
    target_link_libraries(${test} ${target_static} ${GTEST_BOTH_LIBRARIES}) ## 调用此规则时，静态库已经生成，直接引用变量名有更好的跨平台性
    add_test(test_sg_${test} ${test})
endforeach(test)
