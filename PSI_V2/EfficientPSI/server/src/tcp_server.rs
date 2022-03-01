use std::net::{TcpListener, TcpStream};

pub const ADRESSE_PORT: &str = "localhost:1234";

pub const POINT_SIZE: usize = 33;

// FONCTIONS RÉSEAUX

pub fn connect_server() -> Option<TcpStream> {
    let listener = TcpListener::bind(ADRESSE_PORT).expect("Error binding socket");
    match listener.accept() {
        Ok((client, _addr)) => Some(client),
        Err(e) => {
            eprintln!("Error : {}", e);
            None
        }
    }
}
