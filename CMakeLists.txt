cmake_minimum_required(VERSION 3.7)

project(Witch_Sphere LANGUAGES CXX)

add_compile_options(-Wall -Wextra -O3 -ffast-math -flto -march=native)

include_directories("Lib")
file(GLOB_RECURSE SRC RELATIVE ${CMAKE_SOURCE_DIR} "Source/*.cpp" "Lib/*.h")

add_executable(${PROJECT_NAME} ${SRC})
target_link_libraries(${PROJECT_NAME})

install(TARGETS ${PROJECT_NAME} DESTINATION bin)
