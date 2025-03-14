# Assignment 3 - Threads and Processes
## CIS\*3110/Operating Systems W25
### Name: Betty Vuong
### Date: 03/17/25
### Student ID: 1271673

--------------------------------------------------------------------
The program simulates a token ring network using threads and semaphores to manage the communication of the nodes from the token ring. Each node is a thread of the process where the semaphores control the communication. For each thread to be in control and access the data packet, the thread must have the token where token = '1', indicating that it is the threads turn and could utilize send_pkt() if the thread choses to do so according to the data to transmit the data packet. If the thread is transmitting the send_pkt(), token_node() will loop and call send_pkt() until there is no data left to transmit ensuring synchronization. Otherwise the thread will pass the byte through the shared memory using send_byte() and retransmitting the bytes through the switch cases in token_node(). Throughout the checks, once the nodes are all transmitted, the nodes will wait for a terminate flag which will synchronize the parent thread and exit.

Note: The function token_node() has been modified so that the parameters and functuion type would match the parameters of pthread_create. Token node returns a void * type and the parameter is a struct that contains the initial parameters which are TokenRingData and int. The struct is then casted and placed into their following values to function similarily to the model using forks.

To run the simulation, "make" the file first. Then run the executable "./tokensim [number of packets]". If there is a case where the simulation is stuck in a deadlock or some other error, within the makefile, add "-DDEBUG" into the CFLAGS to see the comments and observe where the simulation is stuck at.

## Citations
author: Hamilton-Wright, Andrew
name: semaphore_process_demo
year: 2001
source: Operating Systems Course Directory, School of Computer Science, University of Guleph

author: Hamilton-Wright, Andrew
name: CIS3110-Memory-ForkWaitExec-handout
year: 2025/02/03
source: Operating Systems Course Content, School of Computer Science, University of Guleph

author: Hamilton-Wright, Andrew
name: semaphorep_threads_demo.c
year: 2001
source: Operating Systems Course Content, School of Computer Science, University of Guleph

author: Hamilton-Wright, Andrew
name: thread_children.c
year: 2001
source: Operating Systems Course Content, School of Computer Science, University of Guleph

author: Marshall, Dave
name: IPC:Shared Memory
year: 1999/01/05
source: https://users.cs.cf.ac.uk/Dave.Marshall/C/node27.html