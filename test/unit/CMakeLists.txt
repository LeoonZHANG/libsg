set(tests
    proc
    base64
    charset
    ## 其它的测试程序完成后可以加到这里，比如： http_client http_server img_sdl kcp_client mysql mongodb redis rwlock nats_client
    )

foreach(test ${tests})
    add_executable(${test} ${test}.c)
    target_link_libraries(${test} ${target_static}) ## 调用此规则时，静态库已经生成，直接引用变量名有更好的跨平台性
endforeach(test)
