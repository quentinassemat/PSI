use crate::lib::*;
use crate::tcp_server::*;
use std::io::{Read, Write};

use crypto::digest::Digest;
use crypto::sha3::Sha3;
use gmp::mpz::*;

use std::net::TcpStream;
use std::time::Instant;

mod lib;
mod tcp_server;

// Paramètre RSA (3072-bits security : sécurité équivalente a 265-bits en ECC)
const N_STR: &str = "5809605995369947742926312216170005164844378064778228535629238937639786951989892869279955336051666029612133792076414546616494040251653859079401609619855813190803286365180811524941562947443566728775308677457492383278745587835209840960129766968128129155054494490540839464689153965331222734432083608114126625424443495253488878158403747460860271058528451412608848696587790020047014416862922873581994529584504604163729869034058942640511116557042786110638278145796935137866644588519499305524389454438023956550033738701341712937782797704927990608932915747696494398566405169563628182145314037921576357630692627430511064478043135518212351692818080960037795287906384494979022678011713500074664479689800064472778105850569052030475526680657689806584510916058462620053231871462248188621391195450266116084126693381235573194663489109197795057134753909530421993714563187195854160357423840049708702486896265741345868484806153058069249645346817";
const E_STR: &str =
    "111896287529965718957118242860330879451429556997508021516180302433803091771113";
const D_STR: &str = "2397164355182639256745722021234405290702719723392354103463952989382609666927629490467889357100851087367313422535628065289692021259354292246086881900493068379232802669423113562050747889307703410463088356941253521046151234406204163281233444880326264133217124447076413943962983980837989676559636383999140090141376101461834552392586821993511607837201778618789814845838337965931168538246305461418370068173817352220940511307179318739453373291125550969534812497807318677734777083712267887055814372236032575262969601314666213949043619154041776781465173621419580957032410576662054124567879802816178162625036684366943911687311777119124197772057156192014355255425183902160865304016285703674921488200265272571440094727561261739619043365396137980475617337725779270225629217404815957960587489321066625363150935508291292061254498797288308359207515769011692310210214685356512835551260874715251121061574292105313486359035369762156025630317783";

const FILENAME: &str = "../../sets/mod_dataset5000:1d";

fn main() {
    // récupération du précalcul
    let values = file_to_values(FILENAME);

    // récupération de la clef privée/clef publique
    let d: Mpz = Mpz::from_str_radix(D_STR, 10).unwrap();
    let n: Mpz = Mpz::from_str_radix(N_STR, 10).unwrap();
    let e: Mpz = Mpz::from_str_radix(E_STR, 10).unwrap();

    println!("Server private key\nD : {:?}", d);
    println!("Server public key :\nN : {:?}\nE : {:?}", n, e);

    let now = Instant::now();
    print!("Round 0 : OFFLINE... ");
    // Round 0 : Précalcul
    let mut t: Vec<[u8; 32]> = Vec::new();
    let mut ksj: Mpz;
    for hsj in values.iter() {
        ksj = hsj.powm(&d, &n);
        let mut hasher = Sha3::sha3_256();
        hasher.input(&mpz_to_vec(&ksj));
        let mut tj = [0u8; 32];
        hasher.result(&mut tj);
        t.push(tj);
    }
    let elapsed_time = now.elapsed();
    println!("took {} ms.", elapsed_time.as_millis());

    // mise en place TCP
    println!("Ready TCP");
    let mut stream = connect_server().unwrap();

    // Round 1 : Communication
    let now = Instant::now();
    print!("Round 1... ");
    let mut y = round1(&mut stream);
    let elapsed_time = now.elapsed();
    println!("took {} ms.", elapsed_time.as_millis());

    // Round 2 : Server
    let now = Instant::now();
    print!("Round 2... ");
    for yi in y.iter_mut() {
        *yi = yi.powm(&d, &n);
    }
    let elapsed_time = now.elapsed();
    println!("took {} ms.", elapsed_time.as_millis());

    // Round 3 : Communication
    let now = Instant::now();
    print!("Round 3... ");
    round3(&mut stream, &y, &t);
    let elapsed_time = now.elapsed();
    println!("took {} ms.", elapsed_time.as_millis());

    // Round 4 : Client
    let now = Instant::now();
    print!("Round 4... ");
    let elapsed_time = now.elapsed();
    println!("took {} ms.", elapsed_time.as_millis());
}

pub fn round1(stream: &mut TcpStream) -> Vec<Mpz> {
    let mut bytes_recieved = 0u128;
    // on reçoit la taille
    let mut v_bytes = [0u8; 8];
    bytes_recieved += 8u128;
    stream.read_exact(&mut v_bytes).expect("Error reading");
    let v = u64::from_be_bytes(v_bytes);
    // on reçoit les données
    let mut y: Vec<Mpz> = Vec::new();
    for _ in 0..v {
        let mut yi_bytes = [0u8; MOD_SIZE];
        stream.read_exact(&mut yi_bytes).expect("Error reading");
        bytes_recieved += MOD_SIZE as u128;
        let yi_slice: &[u8] = &yi_bytes;
        y.push(Mpz::from(yi_slice));
    }
    print!("bytes recieved : {} ", bytes_recieved);
    y
}

pub fn round3(stream: &mut TcpStream, y: &[Mpz], t: &[[u8; 32]]) {
    let mut bytes_sent = 0u128;
    // On envoie la taille de t aka w
    bytes_sent += 8u128;
    stream
        .write_all(&(t.len() as u64).to_be_bytes())
        .expect("Errror writing");
    // On envoie les données
    for yi in y.iter() {
        bytes_sent += MOD_SIZE as u128;
        stream.write_all(&mpz_to_vec(yi)).expect("Error writing");
    }
    for ti in t.iter() {
        bytes_sent += 32;
        stream.write_all(ti).expect("Error writing");
    }
    print!("bytes sent : {} ", bytes_sent);
}
