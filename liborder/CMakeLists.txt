include_directories(order)
file(GLOB SOURCES src/*.cpp)
add_library(order STATIC ${SOURCES})
