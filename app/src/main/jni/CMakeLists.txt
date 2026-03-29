cmake_minimum_required(VERSION 3.10.2)

project("MyMod")

# Aquí le decimos que use el archivo Main.cpp
add_library(MyMod SHARED Main.cpp)

# Librerías necesarias de Android
target_link_libraries(MyMod android log)
