use cellular_textures::generate_texture;
use cellular_textures::Config;
use cellular_textures::DistanceOperation;
use cellular_textures::ImageDimensions;
use criterion::BenchmarkId;
use criterion::Criterion;
use criterion::{criterion_group, criterion_main};

pub fn benchmark_with_variable_dimension(c: &mut Criterion) {
    for n in [128, 256, 512, 1024, 2048, 4096] {
        let config = Config::new(
            ImageDimensions::new(n, n),
            false,
            1,
            1000,
            DistanceOperation::Add,
            std::path::PathBuf::from("foo.png"),
        );
        c.bench_with_input(
            BenchmarkId::new("generate_texture_with_variable_dimension", &config),
            &config,
            |b, config| {
                b.iter(|| generate_texture(config));
            },
        );
    }
}

pub fn benchmark_with_variable_num_neighbors(c: &mut Criterion) {
    for n in [1, 10, 100, 1000, 10000] {
        let config = Config::new(
            ImageDimensions::new(1024, 1024),
            false,
            n,
            1_000_000,
            DistanceOperation::Add,
            std::path::PathBuf::from("foo.png"),
        );
        c.bench_with_input(
            BenchmarkId::new("generate_texture_with_variable_num_neighbors", &config),
            &config,
            |b, config| {
                b.iter(|| generate_texture(config));
            },
        );
    }
}

pub fn benchmark_with_variable_num_texture_points(c: &mut Criterion) {
    for n in [1, 10, 100, 1000, 10000] {
        let config = Config::new(
            ImageDimensions::new(1024, 1024),
            false,
            1,
            n,
            DistanceOperation::Add,
            std::path::PathBuf::from("foo.png"),
        );
        c.bench_with_input(
            BenchmarkId::new("generate_texture_with_variable_num_texture_points", &config),
            &config,
            |b, config| {
                b.iter(|| generate_texture(config));
            },
        );
    }
}

criterion_group!(
    benches,
    benchmark_with_variable_dimension,
    benchmark_with_variable_num_neighbors,
    benchmark_with_variable_num_texture_points
);
criterion_main!(benches);
