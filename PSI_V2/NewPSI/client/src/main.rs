use crate::lib::*;
use crate::tcp_client::*;
use std::io::{Read, Write};

use k256::{CompressedPoint, ProjectivePoint, Scalar};

use std::time::Instant;

use crypto::digest::Digest;
use crypto::sha3::Sha3;
use group::GroupEncoding;
use std::net::TcpStream;

mod lib;
mod tcp_client;

const FILENAME: &str = "../../sets/ecc_dataset5000:16";

fn main() {
    // récupération du précalcul
    let keys = file_to_keys(FILENAME);
    let values = file_to_values(FILENAME);

    // mise en place TCP
    let mut stream = connect_client().unwrap();

    // reception clef publique
    let mut pub_key_bytes = [0u8; POINT_SIZE];
    stream
        .read_exact(&mut pub_key_bytes)
        .expect("Error reading");
    let pub_key = ProjectivePoint::from_bytes(CompressedPoint::from_slice(&pub_key_bytes)).unwrap();
    println!("Server public key : {:?}", pub_key.to_affine());

    // Round 1
    let now = Instant::now();
    print!("Round 1... ");
    let mut alpha: Vec<Scalar> = Vec::new();
    let mut y: Vec<ProjectivePoint> = Vec::new();
    for i in 0..keys.len() {
        let rng = rand::thread_rng();
        let exp = Scalar::generate_vartime(rng);
        alpha.push(exp);
        y.push(values.get(i).expect("Out of bound") + &(pub_key * exp));
    }
    let elapsed_time = now.elapsed();
    println!("took {} ms.", elapsed_time.as_millis());

    // Round 2 : Communication
    let now = Instant::now();
    print!("Round 2... ");
    round2(&mut stream, &y);
    let elapsed_time = now.elapsed();
    println!("took {} ms.", elapsed_time.as_millis());

    // Round 3 : Server
    let now = Instant::now();
    print!("Round 3... ");
    let elapsed_time = now.elapsed();
    println!("took {} ms.", elapsed_time.as_millis());

    // Round 4 : Communication
    let now = Instant::now();
    print!("Round 4... ");
    let (w, d, y2, t) = round4(&mut stream, y.len() as u64);
    let elapsed_time = now.elapsed();
    println!("took {} ms.", elapsed_time.as_millis());

    // Round 5
    let now = Instant::now();
    print!("Round 5... ");
    let mut t2: Vec<[u8; 32]> = Vec::new();
    for i in 0..y2.len() {
        let mut hasher = Sha3::sha3_256();
        hasher.input(&(y2[i] - (ProjectivePoint::GENERATOR * alpha[i])).to_bytes());
        hasher.input(&[d]);
        let mut t2i = [0u8; 32];
        hasher.result(&mut t2i);
        t2.push(t2i);
    }
    let elapsed_time = now.elapsed();
    println!("took {} ms.", elapsed_time.as_millis());

    // Output :
    println!("Output :");
    for i in 0..y.len() {
        for j in 0..w {
            if t2[i] == t[j as usize] {
                // println!("key : {}, value : {:?}\n", keys[i], values[i]);
                println!("key : {}", hex::encode(keys[i].to_be_bytes()));
            }
        }
    }
}

pub fn round2(stream: &mut TcpStream, y: &[ProjectivePoint]) {
    let mut bytes_sent = 0u128;
    // On envoie la taille de y aka v
    bytes_sent += 8u128;
    stream
        .write_all(&(y.len() as u64).to_be_bytes())
        .expect("Errror writing");
    // On envoie les données
    for yi in y.iter() {
        bytes_sent += POINT_SIZE as u128;
        stream.write_all(&yi.to_bytes()).expect("Error writing");
    }
    print!("bytes sent : {} ", bytes_sent);
}

pub fn round4(stream: &mut TcpStream, v: u64) -> (u64, u8, Vec<ProjectivePoint>, Vec<[u8; 32]>) {
    let mut bytes_recieved = 0u128;
    // on reçoit la taille
    let mut w_bytes = [0u8; 8];
    bytes_recieved += 8u128;
    stream.read_exact(&mut w_bytes).expect("Error reading");
    let w = u64::from_be_bytes(w_bytes);
    // on reçoit les données
    let mut d = [0u8; 1];
    stream.read_exact(&mut d).expect("Error reading");
    let mut y: Vec<ProjectivePoint> = Vec::new();
    for _ in 0..v {
        bytes_recieved += POINT_SIZE as u128;
        let mut yi_bytes = [0u8; POINT_SIZE];
        stream.read_exact(&mut yi_bytes).expect("Error reading");
        y.push(ProjectivePoint::from_bytes(CompressedPoint::from_slice(&yi_bytes)).unwrap());
    }
    let mut t: Vec<[u8; 32]> = Vec::new();
    for _ in 0..w {
        bytes_recieved += 32;
        let mut ti = [0u8; 32];
        stream.read_exact(&mut ti).expect("Error reading");
        t.push(ti)
    }
    print!("bytes recieved : {} ", bytes_recieved);
    (w, d[0], y, t)
}
