cmake_minimum_required(VERSION 3.10)
project(Roasty)

############################## Custom build modes ###############################

set(CMAKE_CXX_FLAGS_SANITIZE "-fno-omit-frame-pointer -fsanitize=address,signed-integer-overflow,null,alignment,bounds,function,return,vla-bound -O0 -g"
  CACHE STRING
  "Flags used by the C++ compiler during Sanitize builds."
  FORCE)
set(CMAKE_C_FLAGS_SANITIZE "-fno-omit-frame-pointer -fsanitize=address,signed-integer-overflow,null,alignment,bounds,function,return,vla-bound -O0 -g"
  CACHE STRING
  "Flags used by the C compiler during Sanitize builds."
  FORCE)
set(CMAKE_EXE_LINKER_FLAGS_SANITIZE
  ${CMAKE_EXE_LINKER_FLAGS_DEBUG} CACHE STRING
  "Flags used for linking binaries during Sanitize builds."
  FORCE)
set(CMAKE_SHARED_LINKER_FLAGS_SANITIZE
  ${CMAKE_SHARED_LINKER_FLAGS_DEBUG} CACHE STRING
  "Flags used by the shared libraries linker during Sanitize builds."
  FORCE)
mark_as_advanced(
  CMAKE_CXX_FLAGS_SANITIZE		  CMAKE_EXE_LINKER_FLAGS_SANITIZE
  CMAKE_C_FLAGS_SANITIZE		  CMAKE_SHARED_LINKER_FLAGS_SANITIZE
  )

set(CMAKE_BUILD_TYPE "${CMAKE_BUILD_TYPE}" CACHE STRING
  "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel Sanitize."
  FORCE)

############################### External Projects ###############################

include(ExternalProject)

ExternalProject_Add(catch2
	URL https://github.com/catchorg/Catch2/archive/refs/tags/v2.13.7.tar.gz
  DOWNLOAD_DIR $ENV{HOME}/.cmake-downloads
	CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${Roasty_BINARY_DIR}/deps -DCATCH_BUILD_TESTING=NO -DBUILD_TESTING=NO -DCATCH_ENABLE_WERROR=NO -DCATCH_INSTALL_DOCS=NO -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER} -DCMAKE_CXX_FLAGS=${DCMAKE_CXX_FLAGS}
  )

ExternalProject_Add(mongoose
	URL https://github.com/cesanta/mongoose/archive/refs/tags/7.4.tar.gz
  CONFIGURE_COMMAND ""
  BUILD_COMMAND ""
  INSTALL_COMMAND ${CMAKE_COMMAND} -E make_directory ${Roasty_BINARY_DIR}/deps/include
  COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/mongoose.h ${Roasty_BINARY_DIR}/deps/include/
  COMMAND ${CMAKE_COMMAND} -E make_directory ${Roasty_BINARY_DIR}/deps/lib
  COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/mongoose.c ${Roasty_BINARY_DIR}/deps/lib/
  )

ExternalProject_Add(json
	URL https://github.com/nlohmann/json/archive/refs/tags/v3.10.2.tar.gz
	CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${Roasty_BINARY_DIR}/deps -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER} -DCMAKE_CXX_FLAGS=${DCMAKE_CXX_FLAGS} -DJSON_BuildTests=OFF
  )

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

#################################### Targets ####################################
include_directories(PUBLIC ${Roasty_BINARY_DIR}/deps/include)

# Subdirectory where any implementation files are defined
set(ImplementationFiles
    Source/Roasty.cpp
    Source/Server/RoastyServer.cpp
    Source/Storage/DiskStorage.cpp
    Source/Serialisation.cpp
    Source/Model/RoastyModel.cpp
)

set(ExecutableFiles Source/main.cpp)

set(TestFiles Tests/RoastyTests.cpp Tests/SerialisationTests.cpp Tests/RoastTests.cpp)

add_executable(Roasty ${ImplementationFiles} ${ExecutableFiles})
target_link_libraries(Roasty PRIVATE Threads::Threads)
set_property(TARGET Roasty PROPERTY CXX_STANDARD 17)
target_include_directories(Roasty SYSTEM PUBLIC ${Roasty_BINARY_DIR}/deps/include)
add_dependencies(Roasty mongoose json)
target_compile_definitions(Roasty PUBLIC ROASTY_SOURCE_DIR=${Roasty_SOURCE_DIR})
if(WIN32)
  target_link_libraries(Roasty PRIVATE ws2_32)
endif(WIN32)


add_executable(Tests ${ImplementationFiles} ${TestFiles})
target_link_libraries(Tests PRIVATE Threads::Threads)
set_property(TARGET Tests PROPERTY CXX_STANDARD 17)
target_include_directories(Tests SYSTEM PUBLIC ${Roasty_BINARY_DIR}/deps/include)
add_dependencies(Tests catch2 json)
if(WIN32)
  target_link_libraries(Tests PRIVATE ws2_32)
endif(WIN32)
