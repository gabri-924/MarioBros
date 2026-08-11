cmake_minimum_required(VERSION 3.28)
project(CMakeSFMLProject LANGUAGES CXX)

set(CMAKE_POLICY_VERSION_MINIMUM 3.5 CACHE STRING "Minimum policy version for all subprojects")
project(SFMLOpenGLExample LANGUAGES CXX)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include(FetchContent)
FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 3.0.2
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL
    SYSTEM)
FetchContent_MakeAvailable(SFML)

SET( tap1 "01-tappa" )
add_executable( ${tap1} ${tap1}/${tap1}.cpp )
target_link_libraries( ${tap1} PRIVATE SFML::Graphics)

SET( tap2 "02-tappa" )
add_executable( ${tap2} ${tap2}/${tap2}.cpp )
target_link_libraries( ${tap2} PRIVATE SFML::Graphics)