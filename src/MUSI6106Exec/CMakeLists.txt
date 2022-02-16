<<<<<<< HEAD
cmake_minimum_required(VERSION 3.22.1)

=======
>>>>>>> 951885e5c9426d2119789512d6a3af6be5e60cf5
set (APP MUSI6106Exec)

set (AudioLIB AudioFileIO)
## get all source files (note: this is considered bad style, one could also add file names individually)
file(GLOB ${APP}_SOURCES RELATIVE ${CMAKE_SOURCE_DIR}/src/${APP} *.c;*.cpp;*.txt)
file(GLOB ${APP}_HEADERS RELATIVE ${CMAKE_SOURCE_DIR}/src/${APP} *.h;../inc/*.h)

## add include directories
add_inc_directories ()

## create the target
add_executable(${APP} ${${APP}_SOURCES} ${${APP}_HEADERS})

# link the libraries
target_link_libraries(${APP} ${LIB} ${AudioLIB} ${SndLIB} ${OS_LIBS})

