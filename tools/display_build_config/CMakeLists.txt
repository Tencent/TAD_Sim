cmake_minimum_required(VERSION 3.0)
project(build_config)


add_executable(PakMapsConfig
	PakMapsConfig.cpp
	)
target_link_libraries(PakMapsConfig PRIVATE	
)


add_executable(EnableRayTracing
	EnableRayTracing.cpp
	)
target_link_libraries(EnableRayTracing PRIVATE	
)

add_executable(SetLogo
	SetLogo.cpp
	)
target_link_libraries(SetLogo PRIVATE	
)
