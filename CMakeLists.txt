project(gtk-demo C)
cmake_minimum_required(VERSION 2.8)

# Find GTK
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)

# use GTK
include_directories(${GTK3_INCLUDE_DIRS})
link_directories(${GTK3_LIBRARY_DIRS})
add_definitions(${GTK3_CFLAGS_OTHER})

# use my source
add_subdirectory(src)

add_executable(${PROJECT_NAME} ${SOURCE})

# Link
target_link_libraries(${PROJECT_NAME} ${GTK3_LIBRARIES})
