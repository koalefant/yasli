macro(TESTO _library _sources) 

    source_group("testo" FILES "../testo/TestApp.cpp")
    source_group("" FILES CMakeLists.txt)
    source_group("" FILES ${_sources})

    set(_test_sources
        ${_sources}
        "../testo/TestApp.cpp"
        )


    set(_exe_name "test_${_library}")
    add_executable(${_exe_name} WIN32 ${_test_sources})
    target_link_libraries(${_exe_name} ${_library} "utils")

    get_target_property(_exe_location ${_exe_name} LOCATION)
    add_custom_command(TARGET ${_exe_name} POST_BUILD COMMAND ${_exe_location})

endmacro(TESTO)
