cmake_minimum_required(VERSION 3.10)

project(PVFTools)


set(CMAKE_CXX_STANDARD 17)


file(GLOB SRC
	src/*.cpp
	src/*.h
)

add_library(PVFTools STATIC ${SRC})


target_include_directories(PVFTools PUBLIC 
	src 
	../libiconv/include
	../../Engine/Code/Maple/lib/utf8/include
	../../Engine/Code/Maple/lib/charset-detect
	../../Engine/Code/Maple/lib/zlib/src
)
