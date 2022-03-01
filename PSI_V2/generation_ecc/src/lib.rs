use rand::Rng;
use std::collections::HashMap;
use std::fs::File;
use std::io::Write;
use std::io::BufRead;

use hex::encode;

use group::GroupEncoding;

use k256::elliptic_curve::ops::Reduce;
use k256::{ProjectivePoint, Scalar, U256};

use crypto::digest::Digest;
use crypto::sha3::Sha3;

// Scalar <--> FieldBytes (rep binaire)
// ProjectivePoint/AffinePoint <--> CompressedPoint (rep binaire)

// génération de HashMap aléatoire
pub fn random(size: u32) -> HashMap<u64, ProjectivePoint> {
    let mut map: HashMap<u64, ProjectivePoint> = HashMap::new();
    let mut rng = rand::thread_rng();
    let mut key: u64;
    let mut value: ProjectivePoint;
    for _ in 0..size {
        key = rng.gen();
        value = hash_to_curve(&key.to_be_bytes());
        map.insert(key, value);
    }
    map
}

// possible amélioration avec hash2curve mais j'arrive pas le faire marcher
pub fn hash_to_curve(msg: &[u8]) -> ProjectivePoint {
    let mut res = ProjectivePoint::GENERATOR;
    let mut hasher = Sha3::sha3_256();
    hasher.input(msg);
    let mut hash_bytes = [0u8; 32];
    hasher.result(&mut hash_bytes);
    let mul: Scalar = Reduce::from_uint_reduced(U256::from_be_slice(&hash_bytes));
    res *= mul;
    res
}

pub fn hashmap_to_file(file: &mut File, map: HashMap<u64, ProjectivePoint>) {
    for (key, val) in map.iter() {
        file.write_all(encode(&key.to_be_bytes()).as_bytes())
            .expect("Erreur write");
        file.write_all(b" ").expect("Erreur write");
        file.write_all(encode(&val.to_bytes()).as_bytes())
            .expect("Erreur write");
        file.write_all(b"\n").expect("Erreur write");
    }
}

pub fn filename(word: &str) -> String {
    let mut rng = rand::thread_rng();
    let mut res: String = String::from("../sets/ecc_dataset");
    res.push_str(word);
    let random: u8 = rng.gen();
    let random_str: String = encode((random).to_be_bytes());
    res.push(':');
    res.push_str(&random_str);
    res
}
