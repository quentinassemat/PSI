use rand::Rng;
use std::collections::HashMap;
use std::fs::File;
use std::io::Write;

use hex::encode;
use gmp::mpz::*;

use crypto::digest::Digest;
use crypto::sha3::Sha3;

// génération de HashMap aléatoire
pub fn random(size: u32) -> HashMap<u64, Mpz> {
    let mut map: HashMap<u64, Mpz> = HashMap::new();
    let mut rng = rand::thread_rng();
    let mut key: u64;
    let mut value: Mpz;
    for _ in 0..size {
        key = rng.gen();
        value = hash_to_mod(&key.to_be_bytes());
        map.insert(key, value);
    }
    map
}

// possible amélioration avec hash2curve mais j'arrive pas le faire marcher
pub fn hash_to_mod(msg: &[u8]) -> Mpz {
    let mut hasher = Sha3::sha3_512();
    hasher.input(msg);
    let mut hash_bytes = [0u8; 32];
    hasher.result(&mut hash_bytes);
    let hash_slice : &[u8] = &hash_bytes;
    let res = Mpz::from(hash_slice);
    res
}

pub fn hashmap_to_file(file: &mut File, map: HashMap<u64, Mpz>) {
    for (key, val) in map.iter() {
        file.write_all(encode(&key.to_be_bytes()).as_bytes())
            .expect("Erreur write");
        file.write_all(b" ").expect("Erreur write");
        file.write_all(val.to_str_radix(16).as_bytes())
            .expect("Erreur write");
        file.write_all(b"\n").expect("Erreur write");
    }
}

pub fn filename(word: &str) -> String {
    let mut rng = rand::thread_rng();
    let mut res: String = String::from("../sets/mod_dataset");
    res.push_str(word);
    let random: u8 = rng.gen();
    let random_str: String = encode((random).to_be_bytes());
    res.push(':');
    res.push_str(&random_str);
    res
}
