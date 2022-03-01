use std::fs::File;
use std::io::{BufRead, BufReader};

use hex::decode;

use group::GroupEncoding;

use k256::{CompressedPoint, ProjectivePoint};

pub fn file_to_values(filename: &str) -> Vec<ProjectivePoint> {
    let file = File::open(filename).expect("Error opening the file");
    let mut values: Vec<ProjectivePoint> = Vec::new();
    let reader = BufReader::new(file);
    let mut value: ProjectivePoint;
    for line in reader.lines() {
        let line = line.expect("Error reading file\n");
        value = ProjectivePoint::from_bytes(CompressedPoint::from_slice(
            &decode(&line[17..]).expect("Error reading file\n"),
        ))
        .unwrap();
        values.push(value);
    }
    values
}
