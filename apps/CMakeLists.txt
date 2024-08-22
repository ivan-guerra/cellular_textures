find_package(
  Boost
  COMPONENTS program_options
  REQUIRED)

find_package(
  ImageMagick
  COMPONENTS MagickCore Magick++
  REQUIRED)

project(
  ${CMAKE_PROJECT_NAME}
  DESCRIPTION "Cellular Textures"
  LANGUAGES CXX)

add_executable(${PROJECT_NAME})

target_sources(${PROJECT_NAME} PRIVATE ctext.cpp)

target_include_directories(${PROJECT_NAME} PUBLIC ${CTEXT_INCLUDE_DIR})

target_link_libraries(
  ${PROJECT_NAME} PUBLIC texture Boost::program_options ImageMagick::MagickCore
                         ImageMagick::Magick++)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX})
