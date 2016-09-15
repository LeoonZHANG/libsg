################################
# GTest
################################
set(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE})
set(gtest_force_shared_crt ON)
add_subdirectory(gtest)

set(GTEST_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/gtest/googletest/include)
set(GTEST_BOTH_LIBRARIES gtest gtest_main)

################################
# Unit Tests
################################
if(NOT sg_inc_dir)
    set(sg_inc_dir ${CMAKE_CURRENT_SOURCE_DIR}/../../include CACHE PATH "")
endif()
if(NOT sg_lib_dir)
    set(sg_lib_dir ${CMAKE_CURRENT_BINARY_DIR}/../../        CACHE PATH "")
endif()

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
    sym
    net_card
    ## 其它的测试程序完成后可以加到这里，比如： http_client http_server img_sdl kcp_client mysql mongodb redis rwlock nats_client
    )

include_directories(${sg_inc_dir} ${GTEST_INCLUDE_DIRS})

foreach(test ${tests})
    add_executable(${test} ${test}.cpp)
if(WIN32)
    target_link_libraries(${test}
        optimized ${sg_lib_dir}/Release/libsg.lib
        debug ${sg_lib_dir}/Debug/libsg.lib
        ${GTEST_BOTH_LIBRARIES} Rasapi32.lib ws2_32.lib Crypt32.lib)
elseif(APPLE)
    target_link_libraries(${test} ${sg_lib_dir}/libsg.a ${GTEST_BOTH_LIBRARIES} iconv xml2 curl pthread dl)
else()
    target_link_libraries(${test} ${sg_lib_dir}/libsg.a ${GTEST_BOTH_LIBRARIES} pthread dl rt)
endif()
    add_test(test_sg_${test} ${test})
    # Add dependent relative with libsg.a, which prevent failed of parallel make (e.g. make -j4)
    if(TARGET combined)
        add_dependencies(${test} combined)
    endif()
endforeach(test)
