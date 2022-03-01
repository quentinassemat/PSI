use crate::lib::*;
use std::env;
use std::fs::File;
mod lib;



fn main() {
    // prise en compte des arguments
    let args: Vec<String> = env::args().collect();
    if args.len() != 2 {
        eprintln!("Erreur input");
        return;
    }
    let size: u32 = args[1].parse().expect("Erreur input");
    // nom du fichier généré
    let map = random(size);
    let filename = filename(&args[1]);
    let mut file = File::create(filename).expect("Error opening the file");
    hashmap_to_file(&mut file, map);
}