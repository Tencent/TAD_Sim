cmake_minimum_required (VERSION 3.10)

set(CMAKE_CUDA_STANDARD 14)
enable_testing()

if(WIN32)  
set(TENSORRT_PATH $ENV{TENSORRT_PATH})
set(CUDA_PATH $ENV{CUDA_PATH})

else()  
set(TENSORRT_PATH "/usr/local/TensorRT-8.2.1.8")
set(CUDA_PATH "/usr/local/cuda")
set(CMAKE_CUDA_COMPILER /usr/local/cuda/bin/nvcc)
endif()

project(display_cuda_ext LANGUAGES CXX CUDA)

message(STATUS ${CUDA_PATH})
message(STATUS ${TENSORRT_PATH})

find_package(OpenCV CONFIG REQUIRED)
find_package(Boost REQUIRED COMPONENTS system filesystem)
find_package(GTest REQUIRED)


file(GLOB TEST_SRC 
    "src/tests/*.cpp"
)
file(GLOB SRC 
    "src/*.cpp"
    "src/*.cu"
)

include_directories(
    ${CUDA_PATH}/include
    ${TENSORRT_PATH}/include    
    ${GTEST_INCLUDE_DIRS}
    )

add_library(${PROJECT_NAME}  SHARED
    ${SRC}
 )

set(CUDNN cudnn nppc nppig nppicc  nppial nppidei cufft)

find_library(NVINFER NAMES nvinfer PATHS ${TENSORRT_PATH}/lib)
find_library(NVINFERPLUGIN NAMES nvinfer_plugin PATHS ${TENSORRT_PATH}/lib)
find_library(NVONNXPARSERS NAMES nvonnxparser PATHS ${TENSORRT_PATH}/lib)


#target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_11)
#set_target_properties(${PROJECT_NAME}
#    PROPERTIES CUDA_SEPARABLE_COMPILATION ON
#    )


link_directories(
   ${CUDA_PATH}/lib64/
)

target_compile_definitions(${PROJECT_NAME} PUBLIC -DTXSIM_EXPORTS)
target_link_libraries(${PROJECT_NAME} 
    ${CUDNN}
    ${NVINFER}
    ${NVINFERPLUGIN}
    ${NVONNXPARSERS}
    )
    
  
add_executable(${PROJECT_NAME}_onnx_test  
    src/tadsim_trt_engine.cu
    src/tools/onnx_test.cpp)

 
target_link_libraries(${PROJECT_NAME}_onnx_test 
    ${CUDNN}
    ${NVINFER}
    ${NVINFERPLUGIN}
    ${NVONNXPARSERS}
    ${OpenCV_LIBS}
    ${Boost_FILESYSTEM_LIBRARY} 
    ${Boost_SYSTEM_LIBRARY}
    )


add_executable(${PROJECT_NAME}_test  
    ${TEST_SRC}
    ${SRC}
)

 
target_link_libraries(${PROJECT_NAME}_test 
    ${CUDNN}
    ${NVINFER}
    ${NVINFERPLUGIN}
    ${NVONNXPARSERS}
    ${OpenCV_LIBS}
    ${Boost_FILESYSTEM_LIBRARY} 
    ${Boost_SYSTEM_LIBRARY}
    GTest::gmock_main GTest::gmock GTest::gtest
    )
