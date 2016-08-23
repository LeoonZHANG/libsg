set(tests
    proc
    base64
    charset
    ## add other units here, such as: http_client http_server img_sdl kcp_client mysql mongodb redis rwlock nats_client
    )

foreach(test ${tests})
    add_executable(${test} ${test}.c)
    target_link_libraries(${test} ${target_static})
endforeach(test)
