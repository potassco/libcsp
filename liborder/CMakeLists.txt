# [[[source: src
set(ide_source_group "Source Files")
set(source-group
    "${CMAKE_CURRENT_SOURCE_DIR}/src/constraint.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/dlpropagator.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/domain.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/equality.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/linearpropagator.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/normalizer.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/storage.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/translator.cpp")
source_group("${ide_source_group}" FILES ${source-group})
set(source
    ${source-group})
# ]]]

# [[[header: order
set(ide_header_group "Header Files")
set(header-group-order
    "${CMAKE_CURRENT_SOURCE_DIR}/order/config.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/order/configs.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/order/constraint.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/order/dlpropagator.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/order/domain.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/order/equality.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/order/helper.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/order/linearpropagator.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/order/normalizer.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/order/platform.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/order/solver.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/order/storage.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/order/translator.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/order/types.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/order/variable.h")
source_group("${ide_header_group}\\order" FILES ${header-group-order})
set(header
    ${header-group-order})
# ]]]

add_library(liborder ${header} ${source})
target_link_libraries(liborder PUBLIC libpotassco)
target_include_directories(liborder
    PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
    PRIVATE
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/src>")

set_target_properties(liborder PROPERTIES
    OUTPUT_NAME order
    FOLDER lib)

