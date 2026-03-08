cmake_minimum_required(VERSION 3.16)
project(PEA_zad1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(src)

add_executable(pea_zad1
        src/main.cpp
        src/Config.cpp
        src/FileReader.cpp
        src/TourUtils.cpp
        src/RandSolver.cpp
        src/OptTourReader.cpp
        src/NNSolver.cpp
        src/RNNSolver.cpp
        src/BruteForceSolver.cpp
        src/Generator.cpp
        src/CSVWriter.cpp
        src/InstanceListReader.cpp
        src/MemoryUsage.cpp
)