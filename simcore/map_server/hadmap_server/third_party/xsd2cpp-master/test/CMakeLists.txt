# add definition
add_definitions(-DRES_DIR=\"${PROJECT_SOURCE_DIR}/test/resources\")

# create executable
add_executable(LibTest
        LibTest.cpp)

# include directories
target_include_directories(LibTest PRIVATE
        ../src/xsd2cpp_lib)

# link libraries
target_link_libraries(LibTest PRIVATE
        xsd2cpp_lib
        )

# add test
add_gtest(LibTest)
