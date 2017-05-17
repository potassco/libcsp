# [[[source: src
set(ide_source_group "Source Files")
set(source-group
    "${CMAKE_CURRENT_SOURCE_DIR}/src/constrainttest.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/difflogictest.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/domaintest.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/equalitytest.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/linearpropagatortest.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/storagetest.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/translatortest.cpp"
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
set(header-group-test
    "${CMAKE_CURRENT_SOURCE_DIR}/test/mysolver.h")
source_group("${ide_header_group}\\test" FILES ${header-group-test})
set(header
    ${header-group}
    ${header-group-test})
# ]]]

add_executable(test_liborder ${header} ${source})

target_include_directories(test_liborder
    PRIVATE
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>")

target_link_libraries(test_liborder PUBLIC liborder libclasp)
set_target_properties(test_liborder PROPERTIES FOLDER test)

install(TARGETS test_liborder RUNTIME DESTINATION bin)

add_test(NAME test_liborder COMMAND test_liborder)
