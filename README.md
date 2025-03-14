# Assignment 2 - Modelling a Token Ring using Semaphores
## CIS\*3110/Operating Systems W25
### Name: Betty Vuong
### Date: 02/24/25
### Student ID: 1271673

--------------------------------------------------------------------
Each node in the ring is a fork child of the parent process. Where they share a token ring to send memory and indicate flags for the processes. Semaphores control the communications and proccesses which are called by send_pkt(); or send_byte();. For a child to be in control and access the data packet, they must have the token = '1', which indicates that it is the child processes turn. If the child has the token and has data, this means the child is sending the packet and calls send_pkt() which continues to loop through the send_pkt() cases until there is no data left to transmit, this means the token is being held until its given to the next node ensuring synchronization. Otherwise the child is just passing the byte through the shared memory using send_byte() and retransmitting the bytes through the cases in token_node(). During these checks, once everything is sent and received, the nodes will wait for the terminate flag which will kill all the children proccesses. This code is the passive approach.

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