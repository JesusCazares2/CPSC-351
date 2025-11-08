use rand::Rng;
use std::fs::File;
use std::io::{self, BufRead};
use std::sync::{Arc, Mutex};
use std::thread;
use std::time::Duration;

#[derive(Debug)]
struct Process {
    id: usize,
    max: Vec<usize>,
    alloc: Vec<usize>,
    need: Vec<usize>,
    finished: bool,
}

#[derive(Debug)]
struct Bank {
    available: Vec<usize>,
    processes: Vec<Process>,
    total_requests: usize,
    granted_requests: usize,
    denied_requests: usize,
}

impl Bank {
    fn is_safe(&self, pid: usize, request: &[usize]) -> bool {
        let mut work = self.available.clone();
        let mut finish = vec![false; self.processes.len()];
        let mut allocs: Vec<Vec<usize>> = self.processes.iter().map(|p| p.alloc.clone()).collect();
        let mut needs: Vec<Vec<usize>> = self.processes.iter().map(|p| p.need.clone()).collect();

        // Tentatively allocate
        for i in 0..work.len() {
            if request[i] > work[i] {
                return false;
            }
            work[i] -= request[i];
            allocs[pid][i] += request[i];
            needs[pid][i] -= request[i];
        }

        loop {
            let mut found = false;
            for i in 0..finish.len() {
                if !finish[i] && needs[i].iter().zip(&work).all(|(n, w)| n <= w) {
                    for j in 0..work.len() {
                        work[j] += allocs[i][j];
                    }
                    finish[i] = true;
                    found = true;
                }
            }
            if !found {
                break;
            }
        }

        finish.iter().all(|&f| f)
    }

    fn request_resources(&mut self, pid: usize, request: Vec<usize>) -> bool {
        self.total_requests += 1;
        println!("P{} requesting: {:?}", pid, request);

        if self.is_safe(pid, &request) {
            for i in 0..self.available.len() {
                self.available[i] -= request[i];
                self.processes[pid].alloc[i] += request[i];
                self.processes[pid].need[i] -= request[i];
            }
            self.granted_requests += 1;
            println!("Request GRANTED to P{}", pid);
            true
        } else {
            self.denied_requests += 1;
            println!("Request DENIED to P{} (unsafe)", pid);
            false
        }
    }

    fn release_resources(&mut self, pid: usize) {
        println!("P{} has finished. Releasing resources.", pid);
        for i in 0..self.available.len() {
            self.available[i] += self.processes[pid].alloc[i];
        }
        self.processes[pid].finished = true;
    }
}

fn main() -> io::Result<()> {
    let file = File::open("bankInput.txt")?;
    let reader = io::BufReader::new(file);

    let mut lines = reader.lines();
    let first_line = lines.next().unwrap()?.split_whitespace().map(|x| x.parse::<usize>().unwrap()).collect::<Vec<_>>();
    let (p, r) = (first_line[0], first_line[1]);

    let available = lines.next().unwrap()?.split_whitespace().map(|x| x.parse::<usize>().unwrap()).collect::<Vec<_>>();

    let mut processes = Vec::new();
    for (id, line) in lines.enumerate() {
        let nums = line?.split_whitespace().map(|x| x.parse::<usize>().unwrap()).collect::<Vec<_>>();
        let max = nums[0..r].to_vec();
        let alloc = nums[r..2*r].to_vec();
        let need = nums[2*r..3*r].to_vec();
        processes.push(Process { id, max, alloc, need, finished: false });
    }

    let bank = Arc::new(Mutex::new(Bank {
        available,
        processes,
        total_requests: 0,
        granted_requests: 0,
        denied_requests: 0,
    }));

    let mut handles = vec![];

    for pid in 0..p {
        let bank = Arc::clone(&bank);
        let handle = thread::spawn(move || {
            let mut rng = rand::thread_rng();
            loop {
                {
                    let mut bank = bank.lock().unwrap();
                    let proc = &bank.processes[pid];
                    if proc.finished {
                        break;
                    }
                
                    if proc.need.iter().all(|&n| n == 0) {
                        bank.release_resources(pid);
                        break;
                    }
                
                    // Random request that is not all zeros
                    let mut request = vec![0; proc.need.len()];
                    let mut has_request = false;
                    for (i, &need_i) in proc.need.iter().enumerate() {
                        if need_i > 0 {
                            request[i] = rng.gen_range(0..=need_i);
                            if request[i] > 0 {
                                has_request = true;
                            }
                        }
                    }
                
                    if !has_request {
                        // If nothing to request, skip
                        continue;
                    }
                
                    let granted = bank.request_resources(pid, request);
                    
                    if !granted {
                        // After denial, sleep longer
                        drop(bank);
                        thread::sleep(Duration::from_millis(200));
                    }
                }
            }
        });
        handles.push(handle);
    }

    for handle in handles {
        handle.join().unwrap();
    }

    let bank = bank.lock().unwrap();
    println!("All processes completed.");
    println!("Total requests: {}", bank.total_requests);
    println!("Granted requests: {}", bank.granted_requests);
    println!("Denied requests: {}", bank.denied_requests);

    Ok(())
}
