project(
  ${CMAKE_PROJECT_NAME}
  DESCRIPTION "Cellular Textures"
  LANGUAGES CXX)

add_executable(${PROJECT_NAME})

target_sources(${PROJECT_NAME} PRIVATE ctext.cpp)

target_include_directories(${PROJECT_NAME} PUBLIC ${CTEXT_INCLUDE_DIR})

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX})
