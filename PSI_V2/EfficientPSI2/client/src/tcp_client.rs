use std::net::TcpStream;

pub const ADRESSE_PORT: &str = "localhost:1234";

// FONCTIONS RÉSEAUX
pub fn connect_client() -> Option<TcpStream> {
    match TcpStream::connect(ADRESSE_PORT) {
        Ok(stream) => Some(stream),
        Err(e) => {
            eprintln!("Error : {}", e);
            None
        }
    }
}
