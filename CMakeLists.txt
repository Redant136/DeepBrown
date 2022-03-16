cmake_minimum_required(VERSION 3.13)
project(DeepBrown VERSION 1.2)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(APPLE)
  include_directories ( /usr/local/include)
endif()
if(msvc)
SET(GCC_LINK_FLAGS "--config Release")
SET(GCC_COMPILE_FLAGS "--config Release")
else()
SET(GCC_LINK_FLAGS "-O3 -g")
SET(GCC_COMPILE_FLAGS "-O3 -g")
endif()

# set_property(GLOBAL PROPERTY RULE_MESSAGES OFF)

add_executable(DeepBrown main.cpp)

add_subdirectory(kNgine)
target_link_libraries(DeepBrown PRIVATE kNgine)


file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/chess_pieces/ DESTINATION ${CMAKE_BINARY_DIR}/chess_pieces/)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/chess_tock.wav DESTINATION ${CMAKE_BINARY_DIR}/)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/opening.book DESTINATION ${CMAKE_BINARY_DIR}/)


file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/chess_games_dataset/ DESTINATION ${CMAKE_BINARY_DIR}/chess_games_dataset/)

SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${GCC_COMPILE_FLAGS}")
SET(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${GCC_LINK_FLAGS}")
