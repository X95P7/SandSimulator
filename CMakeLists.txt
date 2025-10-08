cmake_minimum_required(VERSION 3.16)
project(SandPhysicsEngine)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenGL REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)

add_executable(SandPhysicsEngine main.cpp)

target_link_libraries(SandPhysicsEngine PRIVATE glfw glad OpenGL::GL)
