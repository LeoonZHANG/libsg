################################
# GTest
################################
find_package(GTest)
if(NOT ${GTEST_FOUND})
    include(ExternalProject)
    ExternalProject_Add(googletest
      URL https://github.com/google/googletest/archive/release-1.8.0.zip
      URL_MD5 adfafc8512ab65fd3cf7955ef0100ff5
      PREFIX ${CMAKE_CURRENT_BINARY_DIR}/gtest
      CMAKE_ARGS -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
                 -Dgtest_force_shared_crt=ON
      INSTALL_COMMAND ""
    )
    ExternalProject_Get_Property(googletest source_dir binary_dir)

    set(GTEST_INCLUDE_DIRS ${source_dir}/googletest/include)
    set(GTEST_LIBRARIES gtest)
    set(GTEST_MAIN_LIBRARIES gtest_main)
    set(GTEST_BOTH_LIBRARIES ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES})
    add_library(${GTEST_LIBRARIES} UNKNOWN IMPORTED)
    add_library(${GTEST_MAIN_LIBRARIES} UNKNOWN IMPORTED)
    if(WIN32)
        set_target_properties(${GTEST_LIBRARIES} PROPERTIES
            IMPORTED_LOCATION_RELEASE ${binary_dir}/googlemock/gtest/Release/${CMAKE_FIND_LIBRARY_PREFIXES}gtest.lib
            IMPORTED_LOCATION_DEBUG ${binary_dir}/googlemock/gtest/Debug/${CMAKE_FIND_LIBRARY_PREFIXES}gtest.lib
            )
        set_target_properties(${GTEST_MAIN_LIBRARIES} PROPERTIES
            IMPORTED_LOCATION_RELEASE ${binary_dir}/googlemock/gtest/Release/${CMAKE_FIND_LIBRARY_PREFIXES}gtest_main.lib
            IMPORTED_LOCATION_DEBUG ${binary_dir}/googlemock/gtest/Debug/${CMAKE_FIND_LIBRARY_PREFIXES}gtest_main.lib
            )
    else()
        set(GTEST_LIBRARIES_PATH ${binary_dir}/googlemock/gtest/${CMAKE_FIND_LIBRARY_PREFIXES}gtest.a)
        set(GTEST_MAIN_LIBRARIES_PATH ${binary_dir}/googlemock/gtest/${CMAKE_FIND_LIBRARY_PREFIXES}gtest_main.a)
        set_target_properties(${GTEST_LIBRARIES} PROPERTIES
            IMPORTED_LOCATION ${GTEST_LIBRARIES_PATH}
            )
        set_target_properties(${GTEST_MAIN_LIBRARIES} PROPERTIES
            IMPORTED_LOCATION ${GTEST_MAIN_LIBRARIES_PATH})
    endif()
    add_dependencies(${GTEST_LIBRARIES} googletest)
    add_dependencies(${GTEST_MAIN_LIBRARIES} googletest)
endif()


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
