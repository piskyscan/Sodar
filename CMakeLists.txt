cmake_minimum_required (VERSION 3.5.1)


project (Sodar)

find_package(ALSA REQUIRED)

add_executable(Sodar Sodar.c )
target_link_libraries(Sodar ${ALSA_LIBRARY} m)
