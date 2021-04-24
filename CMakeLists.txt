cmake_minimum_required(VERSION 3.13)

project(LD48)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake/")

set(CMAKE_VERBOSE_MAKEFILE OFF)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    message("CMAKE_BUILD_TYPE not set, setting it to Release")
    set(CMAKE_BUILD_TYPE Release)
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}") 

############################################################################
# Find OpenMP

find_package(OpenMP REQUIRED)

include_directories(${OpenMP_CXX_NCLUDE_DIRS})


############################################################################
# Find AOgmaNeo include and libs

find_package(AOgmaNeo)

include_directories(${AOGMANEO_INCLUDE_DIRS})


############################################################################
# Find SFML

find_package(SFML 2.5 COMPONENTS system window graphics audio network REQUIRED)

include_directories(${SFML_INCLUDE_DIR})


############################################################################
# Find LDtkLoader

find_package(LDtkLoader)


############################################################################
# Find Box2D

find_package(Box2D)

include_directories(${BOX2D_INCLUDE_DIR})

############################################################################

set(LD48_SRCS
    "source/ltbl/Math.cpp"
    "source/ltbl/quadtree/DynamicQuadtree.cpp"
    "source/ltbl/quadtree/StaticQuadtree.cpp"
    "source/ltbl/quadtree/Quadtree.cpp"
    "source/ltbl/quadtree/QuadtreeNode.cpp"
    "source/ltbl/quadtree/QuadtreeOccupant.cpp"
    "source/ltbl/lighting/LightDirectionEmission.cpp"
    "source/ltbl/lighting/LightPointEmission.cpp"
    "source/ltbl/lighting/LightSystem.cpp"
    "source/Main.cpp"
    "source/world/World.cpp"
    "source/world/Player.cpp"
)

add_executable(LD48 ${LD48_SRCS})

target_link_libraries(LD48 ${OpenMP_CXX_LIBRARIES})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS} -fPIC") # This links -fopenmp

target_link_libraries(LD48 ${AOGMANEO_LIBRARIES})
target_link_libraries(LD48 ${SFML_DEPENDENCIES})
target_link_libraries(LD48 ${SFML_LIBRARIES})
target_link_libraries(LD48 LDtkLoader)
target_link_libraries(LD48 ${BOX2D_LIBRARY})
