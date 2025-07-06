# A Kernel Seedling
This module creates a `/proc/count` entry that outputs the number of running processes on the system. It uses Linux kernel's procfs interface, kernel module loading/unloading, and process table traversal using `for_each_process`.

## Building
```shell
make
```

## Running
```shell
sudo insmod proc_count.ko
cat /proc/count 
```
Result of proc/count was 164


## Cleaning Up
```shell
sudo rmmod proc_count
```

## Testing
```python
python -m unittest
```
Ran 3 tests in 2.243s
OK

Report which kernel release version you tested your module on
(hint: use `uname`, check for options with `man uname`).
It should match release numbers as seen on https://www.kernel.org/.

```shell
uname -r -s -v
```
Linux 5.14.8-arch1-1 #1 SMP PREEMPT Sun, 26, Sep 2021 19:36:15 +0000