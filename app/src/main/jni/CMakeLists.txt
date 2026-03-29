cmake_minimum_required(VERSION 3.10.2)

project("Draken")

# Aquí le decimos que use el archivo Main.cpp
add_library(Draken SHARED Main.cpp)

# Librerías necesarias de Android
target_link_libraries(MyMod android log)
