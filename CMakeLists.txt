project(gtk-demo C)
cmake_minimum_required(VERSION 2.8)

# Find GTK
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK REQUIRED gtk+-3.0)

# Find WebKit
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK REQUIRED webkit2gtk-4.0)

# use GTK
include_directories(${GTK_INCLUDE_DIRS})
link_directories(${GTK_LIBRARY_DIRS})
add_definitions(${GTK_CFLAGS_OTHER})

# use my source
add_subdirectory(src)

add_executable(${PROJECT_NAME} ${SOURCE})

# Link
target_link_libraries(${PROJECT_NAME}
    ${GTK_LIBRARIES}
    -fsanitize=undefined
)
