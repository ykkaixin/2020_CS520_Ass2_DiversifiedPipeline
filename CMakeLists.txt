cmake_minimum_required(VERSION 3.5)
project(Pipeline)
set(CMAKE_CXX_STANDARD 14)
link_directories(~/Desktop/Pipeline2/lib)
add_executable(Pipeline src/pipeLine.c fileprocessor/fileProcess.c src/queue.c src/queue2.h src/queue2.c src/pipeLine.h)
target_link_libraries(Pipeline LINK_PUBLIC instruction)