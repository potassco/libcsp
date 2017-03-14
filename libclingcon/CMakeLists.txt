# [[[source: src
set(ide_source_group "Source Files")
set(source-group
    "${CMAKE_CURRENT_SOURCE_DIR}/src/appsupport.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/clingcondlpropagator.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/clingconorderpropagator.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/solver.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/theoryparser.cpp")
source_group("${ide_source_group}" FILES ${source-group})
set(source
    ${source-group})
# ]]]

# [[[header: clingcon
set(ide_header_group "Header Files")
set(header-group-clingcon
    "${CMAKE_CURRENT_SOURCE_DIR}/clingcon/appsupport.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/clingcon/clingcondlpropagator.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/clingcon/clingconorderpropagator.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/clingcon/solver.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/clingcon/theoryparser.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/clingcon/version.h")
source_group("${ide_header_group}\\clingcon" FILES ${header-group-clingcon})
set(header
    ${header-group-clingcon})
# ]]]

#include_directories(clingcon)
#include_directories(${CMAKE_SOURCE_DIR}/liborder/ ${CMAKE_SOURCE_DIR}/liblp/)
add_library(libclingcon ${header} ${source})
#removed liblp (now libpotassco), should be public by liborder ?
target_link_libraries(libclingcon PUBLIC libclasp liborder)
target_include_directories(libclingcon
    PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
    PRIVATE
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/src>")

set_target_properties(libclingcon PROPERTIES
    OUTPUT_NAME clingcon
    FOLDER lib)
