# CmakeLists.txt in Test dir
# Make sure the compiler can find include files from our Hello library.
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include /usr/local)
# Add binary called "helloWorld" that is built from the source file "test.cc".
# The extension is automatically found.

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")

ADD_EXECUTABLE(newet newet)
# Link the executable to the learning library.
TARGET_LINK_LIBRARIES(newet blitz )

