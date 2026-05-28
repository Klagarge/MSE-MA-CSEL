#import "/doc/metadata.typ": *

= Linux System Optimisation

In this laboratory, the usage of `perf` as tool is experimented. 


== Exercise 1

#task([
Measure the performance of the ex1
],[
```
Performance counter stats for './ex1':

          40609.10 msec task-clock                #    1.000 CPUs utilized          
                22      context-switches          #    0.542 /sec                   
                 0      cpu-migrations            #    0.000 /sec                   
             48867      page-faults               #    1.203 K/sec                  
       33136692484      cycles                    #    0.816 GHz                    
        1671194529      instructions              #    0.05  insn per cycle         
         269592231      branches                  #    6.639 M/sec                  
           1013366      branch-misses             #    0.38% of all branches        

      40.618926728 seconds time elapsed

      39.901620000 seconds user
       0.296158000 seconds sys

```
This program has done 22 context-switches and has 40.6s elapsed.
])

#task([
Which error is in the program of ex1 ?
],[
The error lies in how the array memory is accessed. In C, 2D arrays are stored in a "row-major" order, meaning elements of the same row are contiguous in memory. However, the original code accesses the array using `array[j][i]` within the loops, where `j` (the row) is the inner loop. 

This causes the program to jump across memory addresses non-sequentially, triggering a cache miss almost every time. This can be solved by simply swapping the indices to `array[i][j]` (or swapping the loops) to process memory sequentially:


```c
    int i, j;
    for (i = 0; i < SIZE; i++)
    {
        for (j = 0; j < SIZE; j++)
        {
            array[i][j]+= 10;
        }
    }
```

With these modifications the performance must be a multiple of 10. 

```
 Performance counter stats for './optimized':

            474.62 msec task-clock                #    0.940 CPUs utilized          
                15      context-switches          #   31.604 /sec                   
                 0      cpu-migrations            #    0.000 /sec                   
             48866      page-faults               #  102.959 K/sec                  
         387200454      cycles                    #    0.816 GHz                    
         253128815      instructions              #    0.65  insn per cycle         
          39724528      branches                  #   83.698 M/sec                  
            577317      branch-misses             #    1.45% of all branches        

       0.505146917 seconds time elapsed

       0.233682000 seconds user
       0.237584000 seconds sys

```

This can be observe by doing the same as before with `perf`. Before the time elapsed was around 40s and now about 0.5s. The same observation can be done with the cache missing:
- optimzed  : 753502
- basic     : 406627550


])


#task([
  Show l1 cache missing for ex1 :
],[
  #table(
    columns: (1.5fr, 1fr),
    stroke: none,
    [
      Not optimized
      ```
407036282 L1-dcache-load-misses

39.868545227 seconds time elapsed
39.115950000 seconds user
0.347522000 seconds sys

      ```
    ],[
      Optimzed
      ```
42027157 L1-dcache-load-misses

4.132272210 seconds time elapsed
3.778635000 seconds user
0.296472000 seconds sys
      ```
    ]
  )
  There still is a 10 factor as before between the L1 cache misses.
])


#task([Event analysed with `perf`:],[

- *Instructions*: It indicates the number of cpu instruction done during the program is running.
- *Cache-missing*: This happens when the data used is not currently store in the cache. The ask is passed to the next memory : RAM.
- *Branch-misses*: It happens when there is conditional branch. The CPU tries to predict the next instruction and misses.
- *L1-dcache-load-misses*: It happens when the data is not store in the cache L1. It has the next memory technology, here cache L2.
- *Cpu-migrations*: It indicates the number of times the program has changed of CPU core.
- *Context-switches*: The program is sharing the resource with others. Sometimes, it less the cpu core to another. This involves a context-switching. It has to change some register like the PC.

])


#task([Timing performance of `perf`], [
  There is some executions of the optimized program:

  #figure(table(
    columns: (1fr, 1fr),
    // stroke: none,
    [*Without `perf`*], [*With `perf`*],
    [
      ```
real	0m 4.44s
user	0m 3.83s
sys	  0m 0.29s

      ```
    ],
    [
      ```
real	0m 4.38s
user	0m 4.05s
sys	  0m 0.27s

      ```
    ],[
      ```
real	0m 4.75s
user	0m 4.09s
sys	  0m 0.34s

      ```
    ],[
      ```
real	0m 4.75s
user	0m 4.09s
sys	  0m 0.34s

      ```
    ],
  ),
  caption:[Impact of the tool `perf`]
  )<impact-perf>

  In @impact-perf, the tool does not significantly affect program execution. It is certainly due to the CPU allocations.

])

== Exercise 2

The program fills an array of random between 0 and 512. Then it iterates 10'000 times over all the array to make a sum of all number generated equal or bigger than 256.


#figure(
  table(
    columns:  (1fr),
    [Withtout Optimization],
    [
```

     26170.47 msec task-clock                #    1.000 CPUs utilized          
           17      context-switches          #    0.650 /sec                   
            0      cpu-migrations            #    0.000 /sec                   
           74      page-faults               #    2.828 /sec                   
  21354981945      cycles                    #    0.816 GHz                    
  14768657990      instructions              #    0.69  insn per cycle         
    988541451      branches                  #   37.773 M/sec                  
    327869867      branch-misses             #   33.17% of all branches        

26.178296596 seconds time elapsed

26.117025000 seconds user
  0.003961000 seconds sys
```
    ], [With "sort" optimization],[
      ```
     23430.74 msec task-clock                
           17      context-switches          #    0.726 /sec                   
            0      cpu-migrations            #    0.000 /sec                   
          109      page-faults               #    4.652 /sec                   
  19119368029      cycles                    #    0.816 GHz                    
  14818405467      instructions              #    0.78  insn per cycle         
    997843744      branches                  #   42.587 M/sec                  
       805002      branch-misses             #    0.08% of all branches        

23.439504220 seconds time elapsed

23.382177000 seconds user
  0.003961000 seconds sys
```
    ]
  ),
  caption:[Ex02 timing optimization]
)<sort-optimization>

In @sort-optimization, there is a gain of around 3s due to a massive decrease in branch misses, dropping from 33.17% to 0.08%. 

This is explained by the CPU's Branch Predictor. Inside the loop, the program checks if the value is `>= 256`. When the array is filled with random numbers, the CPU cannot predict the outcome of this condition, resulting in frequent pipeline flushes. However, when the array is sorted, the condition is always false for the first half of the array, and always true for the second half. The CPU easily predicts this pattern, avoiding branch misses and executing much faster.

The same test was done with the `-01` compiler flag and there is almost no difference between the two scipts. The optimzed is around 4.12s and the basic is around 4.6s. The difference of 0.6 sec can be explained with the sort algorithm used in the optimized script, because this is the only difference.


== Exercise 3
By analyzing the call graph with `perf report`, we can trace the indirect calls to `std::operator==<char>` back to our application. The bottleneck originates in the `HostCounter::isNewHost` function, specifically during the `std::find` operation on a `std::vector`:

```c
bool HostCounter::isNewHost(std::string hostname)
{
    return std::find(myHosts.begin(), myHosts.end(), hostname) == myHosts.end();
}
```

Searching through an unsorted vector requires a linear comparison of strings ($O(N)$ complexity), which is highly inefficient. As shown below, processing just a sample of the logs takes over 2 minutes:

```
# time ./read-apache-logs access_log_NASA_Jul95_samples
Processing log file access_log_NASA_Jul95_samples
Found 14867 unique Hosts/IPs
real	2m 15.58s
user	2m 14.68s
sys	0m 0.12s

```
To fix this, the data structure needs to be changed from std::vector to std::set. A set uses a tree-based or hash-based structure, reducing the search complexity to $O(log N)$ or $O(1)$.


#figure(
  image("command-after-optimization.png"),
   caption:[ `perf` report after migrating to `std::set`]
)<command-opti>

After applying the changes, the perf report in @command-opti shows a much healthier execution profile. The execution time drops drastically, creating a massive performance gap compared to the initial vector implementation:
```
# time ./read-apache-logs access_log_NASA_Jul95_samples
Processing log file access_log_NASA_Jul95_samples
Found 14867 unique Hosts/IPs
real	0m 1.55s
user	0m 1.36s
sys	0m 0.10s
```

Even when processing the entire log file containing roughly 2 million entries, the optimized program finishes in under 15 seconds:
```
# time ./read-apache-logs access_log_NASA_Jul95
askljdalksjda
Processing log file access_log_NASA_Jul95
Found 81983 unique Hosts/IPs
real	0m 14.76s
user	0m 13.90s
sys	0m 0.68s
```


#task([Measure interruption latency and jitter], [
  The hardware approach is chosen with an oscilloscope and a square-wave generator. 
  First, the generator toggles a processor pin to trigger the interrupt routine. Then, another pin creates a pulse as a response, which is measured by the oscilloscope. The latency is the delay between the generator's rising edge and the response pulse. The jitter is the variation of this latency over multiple measurements.

  To differentiate between Kernel Space and User Space:
  - *Kernel Space*: The response pin is toggled directly inside the kernel's Interrupt Service Routine (IRQ handler / driver).
  - *User Space*: The response pin is toggled by a user application that wakes up (using `epoll()`) after the kernel has handled the interrupt. 
  
  The difference between these two latency measurements represents the context-switch overhead from kernel mode to user mode.
])
