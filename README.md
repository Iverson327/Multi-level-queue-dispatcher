# Multi-Level-Queue process dispatcher
This is a simulation of process scheduling using 3-level-queue with the first 2 level implementing 
the FCFS (First Come First Serve) scheduling algorithm and RR (Round Robin) for the 3rd level. The 
program will first run the simulation and then compute the performance of the provided combination 
of time-quantums (t0, t1, t2) and starvation count (W). The first level L0 will move the process to 
the end of L1's queue if the running times exceeds t0, same logic for L1. Processes in L2 will be 
moved to the end of the queue in L2 if it exceeds t2. All processes in L1 will be moved to L0 if the 
first process in L1's queue has been waiting for W time after it was last pre-empted. All processes 
in L1 and L2 will be moved to L0 if the first process in L2's queue has been waiting for W time after it 
was last pre-empted. 

## To run the program:
1. Make a binary file process using sigtrap.c
`make process`
2. Make the binary file a2 using a2.c
`make a2`
3. Make a job list in the following format
`<arrival-time, cpu-time, priority>`
e.g.
```
0, 5, 2
1, 5, 1
2, 5, 0
```
4. Run the program with a job list attached
`./a2 <joblist>`
5. Enter the time quantums and W
e.g.
```
Enter t0: 5
Enter t1: 5
Enter t2: 5
Enter W: 5
```
6. remove binary files
`make clean`