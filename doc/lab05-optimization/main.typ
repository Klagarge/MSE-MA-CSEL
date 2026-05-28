#import "/doc/metadata.typ": *

= Optimization

In this laboratory, the usage of `perf` as tool is experimented. 


== Exercise 1

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
The program has 2 loops to go trhough the array. But, there is another loops which encapsulate the 2 others. It involves that the whole array is iterated through 10 times for an addition operation. That's the problem. This can be solve by removing the extren loop and putting a addition of 10:

```c
    int i, j;
    for (i = 0; i < SIZE; i++)
    {
        for (j = 0; j < SIZE; j++)
        {
            array[j][i]+= 10;
        }
    }
```

With these modifications the performance must be a multiple of 10. 

```
 Performance counter stats for './optimized':

           4759.07 msec task-clock                #    0.998 CPUs utilized          
                20      context-switches          #    4.203 /sec                   
                 0      cpu-migrations            #    0.000 /sec                   
             48866      page-faults               #   10.268 K/sec                  
        3883198165      cycles                    #    0.816 GHz                    
         282691820      instructions              #    0.07  insn per cycle         
          40234737      branches                  #    8.454 M/sec                  
            653642      branch-misses             #    1.62% of all branches        

       4.768030627 seconds time elapsed

       4.385881000 seconds user
       0.320226000 seconds sys
```

This can be observe by doing the same as before with `perf`. Before the time elapsed was around 40s and now about 4.7s. The same observation can be done with the cache missing:
- optimzed  : 42103472
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
- *Cpu-migrations*: It indictes the number of times the program has changed of CPU thread.
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

In @sort-optimization, there is a gain of 3s. But, an important augmentation of the branch misses. The rate has decreased from 33.17% (missed) to 0.08%.

The same test was done with the `-01` compiler flag and there is almost no difference between the two scipts. The optimzed is around 4.12s and the basic is around 4.6s. The difference of 0.6 sec can be explained with the sort algorithm used in the optimized script, because this is the only difference.


== Exercise 3


```bash
$ perf record --call-graph dwarf -e cpu-clock -F 75 ./read-apache-logs access_log_NASA_Jul95_samples
Couldn't synthesize bpf events.
Processing log file access_log_NASA_Jul95_samples
Found 14867 unique Hosts/IPs
[ perf record: Woken up 335 times to write data ]
[ perf record: Captured and wrote 83.687 MB perf.data (10269 samples) ]

```

The only line were there is a comparison of 2 element with the `==` operator is :
```c
bool HostCounter::isNewHost(std::string hostname)
{
    return std::find(myHosts.begin(), myHosts.end(), hostname) == myHosts.end();
}
```

The program check if the host is already in the vector. It shows that this operation on `vector` is slow.

```
# time ./read-apache-logs access_log_NASA_Jul95_samples
Processing log file access_log_NASA_Jul95_samples
Found 14867 unique Hosts/IPs
real	2m 15.58s
user	2m 14.68s
sys	0m 0.12s

```

This collections library need to be transform in `set` collections to be faster.