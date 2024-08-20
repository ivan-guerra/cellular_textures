find_package(PNG REQUIRED)
find_package(Boost REQUIRED)

project(
  texture
  DESCRIPTION "Texture Generator"
  LANGUAGES CXX)

add_library(${PROJECT_NAME} STATIC)

target_sources(${PROJECT_NAME} PRIVATE Texture.cpp)

# Had to disable a number of warnings to be able to compile with the third party
# kdtree.h
target_compile_options(
  ${PROJECT_NAME} PUBLIC -Wno-unused-parameter -Wno-return-type
                         -Wno-deprecated-declarations)

target_include_directories(${PROJECT_NAME} PUBLIC ${CTEXT_INCLUDE_DIR})

target_link_libraries(${PROJECT_NAME} PRIVATE Boost::boost PNG::PNG)
