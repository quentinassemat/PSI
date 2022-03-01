use crate::lib::*;
use crate::tcp_client::*;
use std::io::{Read, Write};
use std::time::Instant;

use gmp::mpz::*;


use crypto::digest::Digest;
use crypto::sha3::Sha3;

use std::net::TcpStream;

mod lib;
mod tcp_client;

// Paramètre RSA (3072-bits security : sécurité équivalente a 265-bits en ECC)
const N_STR: &str = "5809605995369947742926312216170005164844378064778228535629238937639786951989892869279955336051666029612133792076414546616494040251653859079401609619855813190803286365180811524941562947443566728775308677457492383278745587835209840960129766968128129155054494490540839464689153965331222734432083608114126625424443495253488878158403747460860271058528451412608848696587790020047014416862922873581994529584504604163729869034058942640511116557042786110638278145796935137866644588519499305524389454438023956550033738701341712937782797704927990608932915747696494398566405169563628182145314037921576357630692627430511064478043135518212351692818080960037795287906384494979022678011713500074664479689800064472778105850569052030475526680657689806584510916058462620053231871462248188621391195450266116084126693381235573194663489109197795057134753909530421993714563187195854160357423840049708702486896265741345868484806153058069249645346817";
const E_STR: &str = "111896287529965718957118242860330879451429556997508021516180302433803091771113";

const FILENAME: &str = "../../sets/mod_dataset5000:74";

fn main() {
    // récupération du précalcul
    let keys = file_to_keys(FILENAME);
    let values = file_to_values(FILENAME);

    // récupération de la clef privée/clef publique
    let n: Mpz = Mpz::from_str_radix(N_STR, 10).unwrap();
    let e: Mpz = Mpz::from_str_radix(E_STR, 10).unwrap();

    println!("Server public key :\nN : {:?}\nE : {:?}", n, e);

    let now = Instant::now();
    print!("Round 0 : OFFLINE... ");
    // Round 0 : Précalcul
   let mut rc : Vec<Mpz> = Vec::new();
   let mut y: Vec<Mpz> = Vec::new();
   for hci in values.iter() {
       let rci = random_mpz(&n);
        y.push((hci * &rci.powm(&e, &n)).modulus(&n));
        rc.push(rci);
   }

   let elapsed_time = now.elapsed();
    println!("took {} ms.", elapsed_time.as_millis());

    // mise en place TCP
    println!("Ready TCP");
    let mut stream = connect_client().unwrap();

    // Round 1 : Communication

    let now = Instant::now();
    print!("Round 1... ");
    round1(&mut stream, &y);
    let elapsed_time = now.elapsed();
    println!("took {} ms.", elapsed_time.as_millis());

    // Round 2 : Server
    let now = Instant::now();
    print!("Round 2... ");
    let elapsed_time = now.elapsed();
    println!("took {} ms.", elapsed_time.as_millis());

    // Round 3 : Communication
    let now = Instant::now();
    print!("Round 3... ");
    let (w, y2, t) = round3(&mut stream, y.len() as u64);
    let elapsed_time = now.elapsed();
    println!("took {} ms.", elapsed_time.as_millis());

    // Round 4 : Client
    let now = Instant::now();
    print!("Round 4... ");
    let mut t2: Vec<[u8; 32]> = Vec::new();
    for i in 0..values.len() {
        let kci = (&y2[i] * ((&rc[i]).invert(&n).expect("Error invert"))).modulus(&n);
        let mut hasher = Sha3::sha3_256();
        hasher.input(&mpz_to_vec(&kci));
        let mut tj = [0u8; 32];
        hasher.result(&mut tj);
        t2.push(tj);
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

pub fn round1(stream: &mut TcpStream, y: &[Mpz]) {
    let mut bytes_sent = 0u128;
    // On envoie la taille de y aka v
    bytes_sent += 8u128;
    stream
        .write_all(&(y.len() as u64).to_be_bytes())
        .expect("Errror writing");
    // On envoie les données
    for yi in y.iter() {
        stream.write_all(&mpz_to_vec(&yi)).expect("Error writing");
        bytes_sent += MOD_SIZE as u128;
    }
    print!("bytes sent : {} ", bytes_sent);
}

pub fn round3(stream: &mut TcpStream, v: u64) -> (u64, Vec<Mpz>, Vec<[u8; 32]>) {
    let mut bytes_recieved = 0u128;
    // on reçoit la taille
    let mut w_bytes = [0u8; 8];
    bytes_recieved += 8;
    stream.read_exact(&mut w_bytes).expect("Error reading");
    let w = u64::from_be_bytes(w_bytes);
    // on reçoit les données
    let mut y: Vec<Mpz> = Vec::new();
    for _ in 0..v {
        let mut yi_bytes = [0u8; MOD_SIZE];
        bytes_recieved += MOD_SIZE as u128;
        stream.read_exact(&mut yi_bytes).expect("Error reading");
        let yi_slice: &[u8] = &yi_bytes;
        y.push(Mpz::from(yi_slice));
    }
    let mut t: Vec<[u8; 32]> = Vec::new();
    for _ in 0..w {
        let mut ti = [0u8; 32];
        bytes_recieved += 32;
        stream.read_exact(&mut ti).expect("Error reading");
        t.push(ti);
    }
    print!("bytes recieved : {} ", bytes_recieved);
    (w, y, t)
}