cmake_minimum_required(VERSION 3.8)
project(PoirutilsD2)

message(STATUS "Configuring ${PROJECT_NAME}...")

include(CheckIncludeFiles)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    thirdparty
    )
    
set(PUD2_SOURCE_FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
    )

# Typically you don't care so much for a third party library's tests to be
# run from your own project's code.
set(JSON_BuildTests OFF CACHE INTERNAL "")

# If you only include this third party in PRIVATE source files, you do not
# need to install it when your main project gets installed.
# set(JSON_Install OFF CACHE INTERNAL "")

# Don't use include(nlohmann_json/CMakeLists.txt) since that carries with it
# unintended consequences that will break the build.  It's generally
# discouraged (although not necessarily well documented as such) to use
# include(...) for pulling in other CMake projects anyways.
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/json)

add_executable(${PROJECT_NAME} ${PUD2_SOURCE_FILES})
set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)
target_link_libraries(${PROJECT_NAME} nlohmann_json::nlohmann_json)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD        		 								
    COMMAND ${CMAKE_COMMAND} -E copy_if_different  		 									
        "${PROJECT_SOURCE_DIR}/config.json"
        $<TARGET_FILE_DIR:${PROJECT_NAME}>) 

message(STATUS "Done Configuring ${PROJECT_NAME}...")