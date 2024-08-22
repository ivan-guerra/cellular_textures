find_package(
  Boost
  COMPONENTS program_options
  REQUIRED)

project(
  ${CMAKE_PROJECT_NAME}
  DESCRIPTION "Cellular Textures"
  LANGUAGES CXX)

add_executable(${PROJECT_NAME})
add_executable(benchmark)

target_sources(${PROJECT_NAME} PRIVATE ctext.cpp)
target_sources(benchmark PRIVATE benchmark.cpp)

target_include_directories(${PROJECT_NAME} PUBLIC ${CTEXT_INCLUDE_DIR})
target_include_directories(benchmark PUBLIC ${CTEXT_INCLUDE_DIR})

target_link_libraries(${PROJECT_NAME} PUBLIC texture Boost::program_options)
target_link_libraries(benchmark PUBLIC texture)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX})
install(TARGETS benchmark RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX})
