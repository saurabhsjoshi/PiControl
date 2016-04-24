cmake_minimum_required (VERSION 2.8)
project (PiControl)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -lwiringPi -O3 -Wall")

include_directories(.)
find_package(Threads REQUIRED)
find_package(Boost 1.54.0 COMPONENTS regex system thread coroutine context filesystem date_time REQUIRED)
find_package(CURL)
include_directories(${Boost_INCLUDE_DIR})

add_executable(piserver server.cxx)
target_link_libraries(piserver ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

find_package(PkgConfig REQUIRED)
pkg_search_module(PSphinx REQUIRED "pocketsphinx")
pkg_search_module(BSphinx REQUIRED "sphinxbase")
pkg_search_module(LCurl REQUIRED "libcurl")
add_executable(mic continuous.c)
target_include_directories(mic PUBLIC ${PSphinx_INCLUDE_DIRS} ${BSphinx_INCLUDE_DIRS} ${LCurl_INCLUDE_DIRS})
target_link_libraries(mic ${PSphinx_LIBRARIES} ${BSphinx_LIBRARIES} ${LCurl_LIBRARIES})

#./mic -hmm /usr/local/share/pocketsphinx/model/en-us/en-us -lm ~/voice/6856.lm -dict ~/voice/6856.dic -samprate 16000/8000/48000 -inmic yes