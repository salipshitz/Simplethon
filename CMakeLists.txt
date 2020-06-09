cmake_minimum_required(VERSION 3.16)
project(simplethon)

set(CMAKE_CXX_STANDARD 17)

add_executable(simplethon main.cpp compiler.cpp compiler.h operator.cpp operator.h filereader.cpp filereader.h runner.cpp runner.h import/built_in.cpp import/built_in.h)