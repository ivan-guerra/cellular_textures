use clap::ValueEnum;
use image::{GrayImage, Luma};
use kd_tree::KdPoint;
use rand::{thread_rng, Rng};
use std::convert::TryFrom;
use std::error::Error;
use std::path::PathBuf;

#[derive(Clone, Debug, ValueEnum)]
pub enum DistanceOperation {
    Add,
    Subtract,
    Multiply,
    Divide,
}

pub struct ImageDimensions {
    pub width: u32,
    pub height: u32,
}

impl ImageDimensions {
    pub fn new(width: u32, height: u32) -> ImageDimensions {
        ImageDimensions { width, height }
    }
}

pub struct Config {
    pub dimensions: ImageDimensions,
    pub invert_colors: bool,
    pub num_neighbors: u32,
    pub num_texture_points: u32,
    pub dist_op: DistanceOperation,
    pub output_file: PathBuf,
}

impl Config {
    pub fn new(
        dimensions: ImageDimensions,
        invert_colors: bool,
        num_neighbors: u32,
        num_texture_points: u32,
        dist_op: DistanceOperation,
        output_file: PathBuf,
    ) -> Config {
        Config {
            dimensions,
            invert_colors,
            num_neighbors,
            num_texture_points,
            dist_op,
            output_file,
        }
    }
}

pub struct Pixel {
    pub location: [i32; 2],
    pub grayscale: u8,
}

impl Pixel {
    pub fn new(location: [i32; 2], grayscale: u8) -> Pixel {
        Pixel {
            location,
            grayscale,
        }
    }

    pub fn wrapped_distance(&self, other: &Pixel, dimensions: &ImageDimensions) -> f64 {
        let dx = (f64::from(self.location[0]) - f64::from(other.location[0])).abs();
        let dy = (f64::from(self.location[1]) - f64::from(other.location[1])).abs();

        let width = f64::from(dimensions.width);
        let dx = if dx > width / 2.0 { width - dx } else { dx };

        let height = f64::from(dimensions.height);
        let dy = if dy > height / 2.0 { height - dy } else { dy };

        (dx * dx + dy * dy).sqrt()
    }
}

impl KdPoint for Pixel {
    type Scalar = i32;
    type Dim = typenum::U2;

    fn at(&self, k: usize) -> i32 {
        self.location[k]
    }
}

struct TextureAlgoData {
    min_dist: f64,
    max_dist: f64,
    dist_buffer: Vec<f64>,
}

impl Default for TextureAlgoData {
    fn default() -> Self {
        TextureAlgoData {
            min_dist: f64::INFINITY,
            max_dist: f64::NEG_INFINITY,
            dist_buffer: Vec::new(),
        }
    }
}

fn generate_search_tree(
    num_texture_points: u32,
    dimensions: &ImageDimensions,
) -> Result<kd_tree::KdTree<Pixel>, Box<dyn Error>> {
    let mut rng = thread_rng();
    let search_pixels: Vec<Pixel> = (0..num_texture_points)
        .map(|_| {
            Ok(Pixel::new(
                [
                    i32::try_from(rng.gen_range(0..dimensions.width))?,
                    i32::try_from(rng.gen_range(0..dimensions.height))?,
                ],
                0,
            ))
        })
        .collect::<Result<Vec<Pixel>, Box<dyn Error>>>()?;

    Ok(kd_tree::KdTree::build(search_pixels))
}

fn set_pixel_intensity(
    texture_pixels: &mut [Pixel],
    texture_data: &TextureAlgoData,
    invert_colors: bool,
) -> Result<(), Box<dyn Error>> {
    for (i, pixel) in texture_pixels.iter_mut().enumerate() {
        let normalized_value = (texture_data.dist_buffer[i] - texture_data.min_dist)
            / (texture_data.max_dist - texture_data.min_dist);
        let intensity_value = if invert_colors {
            1.0 - normalized_value
        } else {
            normalized_value
        };

        pixel.grayscale = u8::try_from((intensity_value * 255.0).round() as i64)
            .map_err(|_| "could not convert grayscale value from f64 to u8")?;
    }

    Ok(())
}

pub fn generate_texture(config: &Config) -> Result<Vec<Pixel>, Box<dyn Error>> {
    let mut texture_data = TextureAlgoData::default();
    let mut texture_pixels = Vec::new();
    let search_tree = generate_search_tree(config.num_texture_points, &config.dimensions)?;

    for i in 0..config.dimensions.height {
        for j in 0..config.dimensions.width {
            let pixel = Pixel::new([i32::try_from(j)?, i32::try_from(i)?], 0);
            if let Some(distance) = search_tree
                .nearests(&pixel, config.num_neighbors as usize)
                .iter()
                .map(|neighbor| pixel.wrapped_distance(neighbor.item, &config.dimensions))
                .reduce(|acc, dist| match config.dist_op {
                    DistanceOperation::Add => acc + dist,
                    DistanceOperation::Subtract => acc - dist,
                    DistanceOperation::Multiply => acc * dist,
                    DistanceOperation::Divide => acc / dist,
                })
            {
                texture_data.min_dist = f64::min(texture_data.min_dist, distance);
                texture_data.max_dist = f64::max(texture_data.max_dist, distance);
                texture_data.dist_buffer.push(distance);
                texture_pixels.push(pixel);
            } else {
                return Err(format!(
                    "failed to find any neighbors of the pixel ({}, {})",
                    pixel.location[0], pixel.location[1],
                )
                .into());
            }
        }
    }

    set_pixel_intensity(&mut texture_pixels, &texture_data, config.invert_colors)?;

    Ok(texture_pixels)
}

pub fn run(config: &Config) -> Result<(), Box<dyn Error>> {
    let texture_pixels = generate_texture(config)?;
    let mut img = GrayImage::new(config.dimensions.width, config.dimensions.height);

    img.pixels_mut()
        .zip(texture_pixels)
        .for_each(|(pixel, texture_pixel)| {
            *pixel = Luma([texture_pixel.grayscale]);
        });
    img.save(&config.output_file)?;

    Ok(())
}

#[cfg(test)]
mod test {
    use super::*;

    #[test]
    fn wrapped_distance_computes_correct_distance_without_wrapping() {
        let dimensions = ImageDimensions::new(10, 10);
        let pixel1 = Pixel::new([0, 0], 0);
        let pixel2 = Pixel::new([3, 4], 0);
        assert_eq!(pixel1.wrapped_distance(&pixel2, &dimensions), 5.0);
    }

    #[test]
    fn wrapped_distance_computes_correct_distance_with_wrapping() {
        let dimensions = ImageDimensions::new(10, 10);
        let pixel1 = Pixel::new([0, 0], 0);
        let pixel2 = Pixel::new([9, 9], 0);
        assert_eq!(
            pixel1.wrapped_distance(&pixel2, &dimensions),
            2.0_f64.sqrt()
        );
    }

    #[test]
    fn generate_search_tree_generates_empty_tree() {
        let dimensions = ImageDimensions::new(10, 10);
        let search_tree = generate_search_tree(0, &dimensions);
        assert!(search_tree.is_ok());

        let search_tree = search_tree.unwrap();
        assert!(search_tree.is_empty());
    }

    #[test]
    fn generate_search_tree_generates_tree_with_size_equal_to_num_texture_points() {
        let dimensions = ImageDimensions::new(10, 10);
        let search_tree = generate_search_tree(5, &dimensions);
        assert!(search_tree.is_ok());

        let search_tree = search_tree.unwrap();
        assert_eq!(search_tree.len(), 5);
    }

    #[test]
    fn set_pixel_intensity_raises_error_when_grayscale_value_cannot_be_converted() {
        let mut texture_pixels = vec![Pixel::new([0, 0], 0)];
        // The following texture_data definition guarantees the pixel intensity value will be 5.
        // When we multiply 5 * 255, we'll be outside the bounds of Pixel.grayscale field's u8
        // limit.
        let texture_data = TextureAlgoData {
            min_dist: 0.0,
            max_dist: 1.0,
            dist_buffer: vec![5.0],
        };
        assert!(set_pixel_intensity(&mut texture_pixels, &texture_data, false).is_err());
    }

    #[test]
    fn set_pixel_intensity_correctly_updates_pixel_intensities() {
        let mut texture_pixels = vec![
            Pixel::new([0, 0], 0),
            Pixel::new([1, 0], 0),
            Pixel::new([0, 1], 0),
            Pixel::new([1, 1], 0),
        ];
        let texture_data = TextureAlgoData {
            min_dist: 0.0,
            max_dist: 1.0,
            dist_buffer: vec![0.5, 0.25, 0.75, 1.0],
        };
        set_pixel_intensity(&mut texture_pixels, &texture_data, false).unwrap();
        assert_eq!(texture_pixels[0].grayscale, 128); // 0.5 * 255 = 127.5 -> 128
        assert_eq!(texture_pixels[1].grayscale, 64); // 0.25 * 255 = 63.75 -> 64
        assert_eq!(texture_pixels[2].grayscale, 191); // 0.75 * 255 = 191.25 -> 191
        assert_eq!(texture_pixels[3].grayscale, 255); // 1.0 * 255 = 255 -> 255
    }

    #[test]
    fn generate_textures_returns_error_when_no_neighbors_found() {
        let config = Config::new(
            ImageDimensions::new(10, 10),
            false,
            1,
            0, // No texture points means there are no neighbors to search for.
            DistanceOperation::Add,
            PathBuf::from("/foo/bar/texture.png"),
        );
        assert!(generate_texture(&config).is_err());
    }
}
