# [[[source: src
set(ide_source_group "Source Files")
set(source-group
    "${CMAKE_CURRENT_SOURCE_DIR}/src/clingcontest.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/unitmain.cpp")
source_group("${ide_source_group}" FILES ${source-group})
set(source
    ${source-group})
# ]]]

# [[[header: .
set(ide_header_group "Header Files")
set(header-group
    "${CMAKE_CURRENT_SOURCE_DIR}/catch.hpp")
source_group("${ide_header_group}" FILES ${header-group})
set(header
    ${header-group})
# ]]]

add_executable(test_libclingcon ${header} ${source})

target_include_directories(test_libclingcon
    PRIVATE
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>")

target_link_libraries(test_libclingcon PUBLIC liborder libclasp libclingcon)
set_target_properties(test_libclingcon PROPERTIES FOLDER test)

install(TARGETS test_libclingcon RUNTIME DESTINATION bin)

add_test(NAME test_libclingcon COMMAND test_libclingcon)
