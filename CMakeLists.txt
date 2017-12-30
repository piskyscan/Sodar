cmake_minimum_required (VERSION 3.5.1)


project (Sodar)

find_package(ALSA REQUIRED)

set(CMAKE_BUILD_TYPE Debug)

add_executable(Sodar Sodar.c FFT_code.cpp sodar_processing.c)
add_executable(TestSodar TestSodar.c FFT_code.cpp TestData.c sodar_processing.c)

target_link_libraries(Sodar ${ALSA_LIBRARY} fftw3 m)
target_link_libraries(TestSodar  fftw3 m)

