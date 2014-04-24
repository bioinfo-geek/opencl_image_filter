cmake_minimum_required(VERSION 2.8)

## Compiler settingfs
set(CMAKE_CXX_FLAGS "-ansi -pedantic -Wall -Wextra -O3")

project(opencl_image_filter)

add_executable(opencl_image_filter config_handle.cpp png_handle.cpp main.cpp)

## POST_BUILD command
add_custom_command(TARGET opencl_image_filter POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different
                       ${CMAKE_CURRENT_SOURCE_DIR}/filtre_LoG_opencl.cl
                       $<TARGET_FILE_DIR:opencl_image_filter>
                   )

## Add subdirectorys
add_subdirectory(libpng-1.5.14)

# Répertoire où se trouve le fichier de recherche du package OpenCL
SET (OpenCL_DIR ${CMAKE_CURRENT_SOURCE_DIR})

#Recherche des packets nécessaire pour la compilation
find_package(OpenCL REQUIRED)


## include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libpng-1.5.14)
include_directories(${OPENCL_INCLUDE_DIR})
target_link_libraries(opencl_image_filter png15_static)
target_link_libraries(opencl_image_filter rt)
target_link_libraries(opencl_image_filter ${OPENCL_LIBRARIES} )


## adding dependencies
add_dependencies(opencl_image_filter REQUIRED png15_static)
