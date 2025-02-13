cmake_minimum_required(VERSION 3.28)
project("cpp-skip" CXX)

set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 23)

add_library("skip")
install(TARGETS "skip" CXX_MODULES_BMI)
target_sources("skip" PUBLIC FILE_SET CXX_MODULES FILES
  "src/lib/skip.cpp"
)

add_executable("demo")
target_link_libraries("demo" PRIVATE "skip")
target_sources("demo" PRIVATE
  "src/app/main.cpp"
)
