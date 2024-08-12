# xsd2cpp_lib files
set(SOURCE_FILES
        functions.h
        CodeGenerator.h
        XSDParser.h
        DataType.h
        DataType.cpp
        xsd2cpp.h
        xsd2cpp.cpp
        )

find_package(tinyxml2 CONFIG REQUIRED)
if(WIN32)          
        include_directories(
        ${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include
        )
endif()
        
        
# create xsd2cpp_lib
add_library(xsd2cpp_lib ${SOURCE_FILES})

# link tinyxml
target_link_libraries(xsd2cpp_lib PRIVATE
tinyxml2::tinyxml2)
