# Hash Hash Hash
This lab implements and tests three versions of a hash table: a base serial version, a thread safe version with a global lock, and a thread safe version with fine grained locking. 

## Building
```shell
make
```

## Running
```shell
./hash-table-tester -t [num_threads] -s [inserts_per_thread]
```
results for -t 3 -s 50000: 
Hash table base: 287,477 usec 
- 0 missing
Hash table v1: 314,196 usec
-0 missing
Hash table v2: 259,565 usec
-0 missing

## First Implementation
In the `hash_table_v1_add_entry` function, I added a single pthread_mutex_t lock to the entire hash table structure. 

### Performance
```shell
./hash_table-tester -t 3 -s 50000
```

Version 1 is a little slower/faster than the base version. As expected due to global locking, v1 should run slower than the base implementation however I notice that sometimes it outperforms the base version. This can be attributed to OS scheduling quirks or parallelism at low contention. 

## Second Implementation
In the `hash_table_v2_add_entry` function, I implemented fine grained locking by placing a separate pthread_mutex_t in each bucket of the hash table. 

### Performance
```shell
./hash-table-tester -t 4 -s 50000
```
results for -t 4 -s 50000: 
Hash table base: 530,512 usec 
- 0 missing
Hash table v1: 524,939 usec
-0 missing
Hash table v2: 464,621 usec
-0 missing

At the default thread count, v2 consistently outperforms both the base and v1 implementations. This is because the per-bucket locks reduce contention by a lot. However at higher thread counts performance can dip due to CPU limitations or other things like lock collisions. 

## Cleaning up
```shell
make clean
```