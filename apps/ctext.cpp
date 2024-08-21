#include <boost/program_options.hpp>
#include <cstddef>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <string>
#include <string_view>

#include "Texture.h"

static void PrintErrAndExit(const std::string_view err_msg) {
  std::cerr << "error: " << err_msg << std::endl;
  std::exit(EXIT_FAILURE);
}

int main(int argc, char** argv) {
  namespace po = boost::program_options;
  try {
    std::filesystem::path filepath;
    ctext::TextureConfig conf = {};

    /* clang-format off */
    po::options_description visible_opts("Allowed options");
    visible_opts.add_options()
      ("help,h", "print this help message")
      ("num-points,n",
       po::value<size_t>(&conf.num_points)->default_value(1000),
       "number of texture points")
      ("invert-colors,i",
        po::value<bool>(&conf.invert_colors)->implicit_value(true),
        "invert-pixel-color")
      ("enable-tiling,t",
        po::value<bool>(&conf.is_tiled)->implicit_value(true),
        "tile textures")
      ("num-neighbors,k",
       po::value<size_t>(&conf.num_neighbors)->default_value(1),
       "number of neighboring texture points to consider at each pixel")
      ("dist-op,d",
        po::value<ctext::DistOp>(&conf.op)->default_value(
          ctext::DistOp::kAdd),
        "operation applied to all neighboring point distances: \n"
        "+) Add\n"
        "-) Subtract\n"
        "*) Multiply");

    po::options_description positional_opts("Positional options");
    positional_opts.add_options()
      ("width",
        po::value<size_t>(&conf.dim.width),
        "image width")
      ("height", po::value<size_t>(&conf.dim.height), "image height")
      ("filepath",
        po::value<std::filesystem::path>(&filepath),
        "output PNG filepath");
    /* clang-format on */

    // Specify positional options and their expected order on the cmdline.
    po::positional_options_description p;
    p.add("width", 1);
    p.add("height", 1);
    p.add("filepath", 1);

    po::variables_map vm;
    po::options_description cmdline_options;
    cmdline_options.add(visible_opts).add(positional_opts);
    po::store(po::command_line_parser(argc, argv)
                  .options(cmdline_options)
                  .positional(p)
                  .run(),
              vm);
    po::notify(vm);

    if (vm.count("help")) {
      std::cout << "usage: ctext WIDTH HEIGHT FILEPATH [OPTION]..."
                << std::endl;
      std::cout << visible_opts << std::endl;
      return 0;
    }
    if (!vm.count("width") || !vm.count("height") || !vm.count("filepath")) {
      PrintErrAndExit(
          "missing one or more required arguments (run 'ctext --help')");
    }
    if ((vm["width"].as<size_t>() <= 1) || (vm["height"].as<size_t>() <= 1)) {
      PrintErrAndExit(
          "illegal image width or height, all dimensions must be > 1");
    }
    if (vm.count("num-points") && !vm["num-points"].as<size_t>()) {
      PrintErrAndExit("number of points must be nonzero");
    }

    // Create the texture's pixels and output them to a grayscale PNG.
    const auto pixels = ctext::CreateTexture(conf);
    ctext::WriteToPng(conf, pixels, filepath);
  } catch (const std::exception& e) {
    PrintErrAndExit(e.what());
  } catch (...) {
    PrintErrAndExit("exception of unknown type");
  }
  std::exit(EXIT_SUCCESS);
}
