# create executable
add_executable(xsd2cpp main.cpp)

# link tinyxml2
target_link_libraries(xsd2cpp PRIVATE
        xsd2cpp_lib
        )

# include directories
target_include_directories(xsd2cpp PRIVATE
        ../xsd2cpp_lib)





add_executable(xosc_test 
test.cpp
private/openscenario.cc
)

# include directories
find_package(tinyxml2 CONFIG REQUIRED)
if(WIN32)          
target_include_directories(xosc_test PRIVATE
        ${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include
)
endif()
target_include_directories(xosc_test PRIVATE
        public)
# link tinyxml2
target_link_libraries(xosc_test PRIVATE
        xsd2cpp_lib
        )
