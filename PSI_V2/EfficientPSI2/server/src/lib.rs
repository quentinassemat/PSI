use std::fs::File;
use std::io::{BufRead, BufReader};

use gmp::mpz::*;

pub const MOD_SIZE: usize = 384;

pub fn mpz_to_vec(input: &Mpz) -> [u8; MOD_SIZE] {
    let mut res = [0u8; MOD_SIZE];
    let mut string = input.to_str_radix(16);
    if string.len() % 2 == 1 {
        string.insert(0, '0');
    }
    let vector = hex::decode(string).expect("Error decoding");
    for i in 0..vector.len() {
        res[i] = vector[i];
    }
    res
}

pub fn file_to_values(filename: &str) -> Vec<Mpz> {
    let file = File::open(filename).expect("Error opening the file");
    let mut values: Vec<Mpz> = Vec::new();
    let reader = BufReader::new(file);
    let mut value: Mpz;
    for line in reader.lines() {
        let line = line.expect("Error reading file\n");
        value = Mpz::from_str_radix(&line[17..], 16).unwrap();
        values.push(value);
    }
    values
}
