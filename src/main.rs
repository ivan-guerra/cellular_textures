use cellular_textures::Config;
use cellular_textures::DistanceOperation;
use cellular_textures::ImageDimensions;
use clap::Parser;

#[derive(Parser, Debug)]
#[command(version, about, long_about = None)]
struct Args {
    #[arg(
        value_parser = clap::value_parser!(u32).range(128..=4096),
        help = "image width in pixels"
    )]
    width: u32,

    #[arg(
        value_parser = clap::value_parser!(u32).range(128..=4096),
        help = "image height in pixels"
    )]
    height: u32,

    #[arg(help = "output image path (e.g., /foo/bar/texture.png)")]
    output_file: String,

    #[arg(
        short = 'n',
        long,
        default_value_t = 1000,
        value_parser = clap::value_parser!(u32).range(1..=1_000_000),
        help = "number of texture points"
    )]
    num_texture_points: u32,

    #[arg(
        short = 'p',
        long,
        default_value_t = 1,
        value_parser = clap::value_parser!(u32).range(1..=1_000_000),
        help = "number of neighbors to consider per pixel"
    )]
    num_neighbors: u32,

    #[arg(
        short = 'i',
        long,
        default_value_t = false,
        help = "invert colors of output image"
    )]
    invert_colors: bool,

    #[arg(
        short = 'd',
        long,
        value_enum,
        default_value_t = DistanceOperation::Add,
        help = "distance operation to use"
    )]
    dist_op: DistanceOperation,
}

fn main() {
    let args = Args::parse();
    let config = Config::new(
        ImageDimensions::new(args.width, args.height),
        args.invert_colors,
        args.num_neighbors,
        args.num_texture_points,
        args.dist_op,
        args.output_file.into(),
    );

    if let Err(e) = cellular_textures::run(&config) {
        eprintln!("error: {}", e);
        std::process::exit(1);
    }
}
