use crate::thread_pool::PoolCreationError::{ThreadsNotPositive};
use std::sync::{Arc, Mutex, mpsc};
use std::thread;

// Should use Box because Rust cannot know the trait size in compile time
type Job = Box<dyn FnOnce() + Send + 'static>;

pub struct ThreadPool {
    threads: Vec<Worker>,
    job_queue: mpsc::Sender<Message>
}

struct Worker {
    thread: Option<thread::JoinHandle<()>>,
}

enum Message {
    NewJob(Job),
    Terminate,
}

#[derive(Debug)]
pub enum PoolCreationError {
    ThreadsNotPositive
}


impl ThreadPool {
    pub fn new(size: usize) -> Result<ThreadPool, PoolCreationError> {
        if size <= 0 {
            return Err(ThreadsNotPositive)
        }
        let mut workers = Vec::with_capacity(size);
        let (sender, receiver) = mpsc::channel();
        // sender is thread safe
        // receiver is not thread safe, but using mutex to keep it thread safe
        let receiver = Arc::new(Mutex::new(receiver));
        for id in 0..size {
            workers.push(Worker::new(id, Arc::clone(&receiver)));
        }

        Ok(ThreadPool {
            threads: workers,
            job_queue: sender
        })
    }

    pub fn execute<F>(&self, f: F)
        where F: FnOnce() + Send + 'static {
        self.job_queue.send(Message::NewJob(Box::new(f))).unwrap();
    }
}

impl Drop for ThreadPool {
    fn drop(&mut self) {
        println!("Sending terminate message to all workers...");

        for _ in &mut self.threads {
            self.job_queue.send(Message::Terminate).unwrap();
        }

        println!("Shutting down all workers...");
        for worker in &mut self.threads {
            // take worker.thread out, worker.thread becomes None
            // move inner JointHandler to thread
            if let Some(thread) = worker.thread.take() {
                // join will remove the thread itself
                thread.join().unwrap();
            }
        }
    }
}

impl Worker {
    fn new(id: usize, job_source: Arc<Mutex<mpsc::Receiver<Message>>>) -> Worker {
        // Why using move?
        // If not move, the spawn closure won't gain ownership of job_source, just reference
        // and job_source may invalid => compile error
        // So use move to make the closure gains the ownership of job_source
        let thread = thread::spawn( move || {
            loop {
                let message = job_source.lock().unwrap().recv().unwrap();

                match message {
                    Message::NewJob(job) => {
                        println!("Worker {} running...", id);
                        // job is Box<dyn FnOnce>, just access it as the ref of the FnOnce
                        job();
                    },
                    Message::Terminate => {
                        println!("Worker {} stopping...", id);
                        break;
                    }
                }
            }
            // We don't use while-let, because the MutexGuard<T> is still alive,
            // that will block other thread when executing the job()
        } );
        Worker {
            thread: Some(thread)
        }
    }
}