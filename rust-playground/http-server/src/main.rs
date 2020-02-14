mod thread_pool;

use std::net::{TcpListener, TcpStream};
use std::io::{Read, Write};
use std::time::Duration;
use std::sync::{Arc, RwLock};

fn main() {
    let tcp_listener = TcpListener::bind("localhost:7878")
        .unwrap_or_else(|err| {
            eprintln!("Error occurred: {}", err);
            std::process::exit(1);
        });

    let thread_pool = thread_pool::ThreadPool::new(4).unwrap();

    for stream in tcp_listener.incoming() {
        if let Ok(stream) = stream {
            stream.set_nodelay(true).expect("Set no-delay failed.");
            let stream = Arc::new(RwLock::new(stream));
            let cloned_stream = Arc::clone(&stream);
            thread_pool.execute(  || {
                handle_connection(cloned_stream);
            });
            println!("Connection established: {}", stream.read().unwrap().peer_addr().unwrap());
        } else {
            eprintln!("Connection failed.");
        }
    }
}


fn handle_connection(stream: Arc<RwLock<TcpStream>>) {
    let mut buffer= [0u8; 512];
    let mut stream = stream.write().unwrap();
    match stream.read(&mut buffer) {
        Ok(_) => {
            let get_root = b"GET / HTTP/1.1\r\n";
            let get_sleep = b"GET /sleep HTTP/1.1\r\n";

            let (status_line, filename) = if buffer.starts_with(get_root) {
                ("HTTP/1.1 200 OK\r\n\r\n", "hello.html")
            } else if buffer.starts_with(get_sleep) {
                std::thread::sleep(Duration::from_secs(5));
                ("HTTP/1.1 200 OK\r\n\r\n", "hello.html")
            } else {
                ("HTTP/1.1 404 NOT FOUND\r\n\r\n", "404.html")
            };

            let contents = std::fs::read_to_string(filename).unwrap();
            let response = format!("{}{}", status_line, contents);

            stream.write(response.as_bytes()).unwrap();
            stream.flush().unwrap();
        },
        Err(err) => {
            eprintln!("Error occurred {}", err);
        }
    }
}