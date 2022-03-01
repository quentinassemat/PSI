use crate::lib::*;
use crate::tcp_server::*;
use std::io::{Read, Write};

use std::time::Instant;

use k256::{CompressedPoint, ProjectivePoint, Scalar};

use crypto::digest::Digest;
use crypto::sha3::Sha3;
use group::GroupEncoding;
use std::net::TcpStream;

mod lib;
mod tcp_server;

const FILENAME: &str = "../../sets/ecc_dataset5000:36";

fn main() {
    // mise en place TCP
    let mut stream = connect_server().unwrap();

    // récupération du précalcul
    let values = file_to_values(FILENAME);

    // Round 1 : Client
    let now = Instant::now();
    print!("Round 1... ");
    let elapsed_time = now.elapsed();
    println!("took {} ms.", elapsed_time.as_millis());

    // Round 2 : Communication
    let now = Instant::now();
    print!("Round 2... ");
    let (x, mut y) = round2(&mut stream);
    let elapsed_time = now.elapsed();
    println!("took {} ms.", elapsed_time.as_millis());

    // Round 3 :
    let now = Instant::now();
    print!("Round 3... ");
    let rng = rand::thread_rng();
    let rs: Scalar = Scalar::generate_vartime(rng);
    let z = ProjectivePoint::GENERATOR * rs;

    let mut t: Vec<[u8; 32]> = Vec::new();
    for item in values.iter() {
        let ksj = (x - item) * rs;
        let mut hasher = Sha3::sha3_256();
        hasher.input(&ksj.to_bytes());
        let mut tj = [0u8; 32];
        hasher.result(&mut tj);
        t.push(tj);
    }

    for item in y.iter_mut() {
        *item *= rs;
    }
    let elapsed_time = now.elapsed();
    println!("took {} ms.", elapsed_time.as_millis());

    // Round 4 : Communication
    let now = Instant::now();
    print!("Round 4... ");
    round4(&mut stream, &z, &y, &t);
    let elapsed_time = now.elapsed();
    println!("took {} ms.", elapsed_time.as_millis());

    // Round 5 : Client
    let now = Instant::now();
    print!("Round 5... ");
    let elapsed_time = now.elapsed();
    println!("took {} ms.", elapsed_time.as_millis());
}

pub fn round2(stream: &mut TcpStream) -> (ProjectivePoint, Vec<ProjectivePoint>) {
    let mut bytes_recieved = 0u128;
    // on reçoit la taille
    bytes_recieved += 8u128;
    let mut v_bytes = [0u8; 8];
    stream.read_exact(&mut v_bytes).expect("Error reading");
    let v = u64::from_be_bytes(v_bytes);
    // on reçoit les données
    let mut x_bytes = [0u8; POINT_SIZE];
    bytes_recieved += POINT_SIZE as u128;
    stream.read_exact(&mut x_bytes).expect("Error reading");
    let x = ProjectivePoint::from_bytes(CompressedPoint::from_slice(&x_bytes)).unwrap();

    let mut y: Vec<ProjectivePoint> = Vec::new();
    for _ in 0..v {
        let mut yi_bytes = [0u8; POINT_SIZE];
        bytes_recieved += POINT_SIZE as u128;
        stream.read_exact(&mut yi_bytes).expect("Error reading");
        y.push(ProjectivePoint::from_bytes(CompressedPoint::from_slice(&yi_bytes)).unwrap());
    }
    print!("bytes recieved : {} ", bytes_recieved);
    (x, y)
}

pub fn round4(stream: &mut TcpStream, z: &ProjectivePoint, y: &[ProjectivePoint], t: &[[u8; 32]]) {
    let mut bytes_sent = 0u128;
    // On envoie la taille de t aka w
    bytes_sent += 8u128;
    stream
        .write_all(&(t.len() as u64).to_be_bytes())
        .expect("Errror writing");
    // On envoie les données
    stream.write_all(&z.to_bytes()).expect("Error writing");
    for yi in y.iter() {
        bytes_sent += POINT_SIZE as u128;
        stream.write_all(&yi.to_bytes()).expect("Error writing");
    }
    for ti in t.iter() {
        bytes_sent += 32;
        stream.write_all(ti).expect("Error writing");
    }
    print!("bytes sent : {} ", bytes_sent);
}
