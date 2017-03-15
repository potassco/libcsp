
add_subdirectory(liborder)
add_subdirectory(libclingcon)
if(CLINGCON_BUILD_TESTS)
    add_subdirectory(tests)
endif()
