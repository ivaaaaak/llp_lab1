cmake_minimum_required(VERSION 3.24)
project(llp_lab1 C)

set(CMAKE_C_STANDARD 11)

add_compile_definitions(_CRT_SECURE_NO_WARNINGS)



add_executable(lab1 
src/database.c
src/graph.c
src/file.c
src/query.c
src/main.c
include/database.h
include/file.h
include/graph.h
include/query.h
test/tests.c
test/tests.h
test/generator.c
test/generator.h
)

