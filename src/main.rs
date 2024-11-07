use cellular_textures::Config;
use cellular_textures::DistanceOperation;
use cellular_textures::ImageDimensions;

fn main() {
    let _config = Config::new(
        ImageDimensions::new(10, 10),
        true,
        1,
        256,
        DistanceOperation::Add,
    );
}
