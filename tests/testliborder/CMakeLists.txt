include_directories(test)
include_directories(${CMAKE_SOURCE_DIR}/liborder/order)
file(GLOB SOURCES src/*.cpp)
add_executable(testliborder ${SOURCES})
find_package(CppUnit)
target_link_libraries(testliborder order libclasp ${CPPUNIT_LIBRARY} dl)