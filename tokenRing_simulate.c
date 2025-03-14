// Betty Vuong
/*
 * The program simulates a Token Ring LAN by forking off a process
 * for each LAN node, that communicate via shared memory, instead
 * of network cables. To keep the implementation simple, it jiggles
 * out bytes instead of bits.
 *
 * It keeps a count of packets sent and received for each node.
 */
#include <stdio.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include "tokenRing.h"

/*
 * This function is the body of a child process emulating a node.
 */
void *
	token_node(arg) void * arg;
{
	int rcv_state = TOKEN_FLAG, not_done = 1, sending = 0, len;
	unsigned char byte;
	threadArg *pass = (threadArg *)arg;
	int num = pass->num;
	struct TokenRingData * control = pass->control;

// TEST
#ifdef DEBUG
	fprintf(stderr, "Node %d: Waiting for token\n", num);
#endif

	/*
	 * If this is node #0, start the ball rolling by creating the
	 * token.
	 */
	if (num == 0)
	{
		send_byte(control, num, '1');
		rcv_state = TOKEN_FLAG;
		control->snd_state = TOKEN_FLAG;
#ifdef DEBUG
		fprintf(stderr, "created first token %d\n", num);
#endif
	}

	/*
	 * Loop around processing data, until done.
	 */
	while (not_done)
	{
		// call to recieve the byte
		byte = rcv_byte(control, num);

		/*
		 * Handle the byte, based upon current state.
		 */
		switch (rcv_state)
		{
		case TOKEN_FLAG:
			if (sending == 1)
			{
				send_pkt(control, num);
				// rcv_state = TOKEN_FLAG;
				if (control->shared_ptr->node[num].to_send.length == 0)
				{
#ifdef DEBUG
					fprintf(stderr, "\npacket sent for node %d\n", num);
#endif
					sending = 0; //reset to indicate exit out of sending the data packet and possibly going into retransmitting
					rcv_state = TOKEN_FLAG; //reset the state
				}
				break;
			}
			if (byte == '1' && control->shared_ptr->node[num].to_send.token_flag == '0')
			{
				sending = 1;
#ifdef DEBUG
				fprintf(stderr, "\ntoken found and data for node %d\n", num);
#endif
				// break out of this since you need to start sending the data
				// prepare for sendpkt
				control->snd_state = TOKEN_FLAG;
				if (control->shared_ptr->node[num].to_send.length == 0)
				{
					rcv_state = TO; //process complete pass to retransmitting bytes
				}
				else
				{
					rcv_state = TOKEN_FLAG; //start sending packet
				}
				send_pkt(control, num);
				break;
			}
			else
			{ //passing the byte along or killing the child process
				sending = 0;
				send_byte(control, num, byte); // send the token away as the data does not need to be transferred
				// check if this is a terminate, kill the child
				if (control->shared_ptr->node[num].terminate == 1)
				{
					not_done = 0; //break the loop
#ifdef DEBUG
					fprintf(stderr, "\nchild killed %d\n", num);
#endif
				}
				else if (byte == '0')
				{ //pass the byte to the next node by retransmitting
#ifdef DEBUG
					fprintf(stderr, "\nsend info\n");
#endif
					rcv_state = TO;
				}
				break;
			}

		case TO:
			// printf("\nreceive %d byte %d\n", num, byte);
			send_byte(control, num, byte);
			rcv_state = FROM;
			//count the received amount
			if (byte == num)
			{ // receiving
				control->shared_ptr->node[num].received++;
			}
#ifdef DEBUG
			fprintf(stderr, "\nto %d\n", num);
#endif
			break;

		case FROM:
			// fprintf(stderr, "\nfrom %d\n", num);
			send_byte(control, num, byte);
			rcv_state = LEN;
#ifdef DEBUG
			fprintf(stderr, "\nfrom %d\n", num);
#endif
			break;

		case LEN:
			// fprintf(stderr, "\nlen %d curr %d\n", num, byte);
			//  get len
			len = byte;
			send_byte(control, num, byte);
#ifdef DEBUG
			fprintf(stderr, "\nlen %d\n", num);
#endif
			rcv_state = DATA;
			break;

		case DATA:
			// fprintf(stderr, "\ndata %d\n", num);
			// determine if looping needs to be done to go byte by byte
			if (len > 0)
			{
				len--;
				send_byte(control, num, byte);
				// printf("\nloop for node %d\n", num);
				rcv_state = DATA;
			}
			else if (len == 0)
			{
				// exit this loop
				send_byte(control, num, byte);
				rcv_state = TOKEN_FLAG; // restart the state
			}
#ifdef DEBUG
			fprintf(stderr, "\ndata %d", num);
#endif
			break;
		};
	}
	return NULL;
}

/*
 * This function sends a data packet followed by the token, one byte each
 * time it is called.
 */
void
	send_pkt(control, num) struct TokenRingData *control;
int num;
{
	static int sndpos, sndlen;
// TEST
#ifdef DEBUG
	fprintf(stderr, "Node %d: Sending packet, state=%d\n", num, control->snd_state);
#endif

	switch (control->snd_state)
	{
	case TOKEN_FLAG:
// WAIT_SEM(control,CRIT);
// TEST
#ifdef DEBUG
		printf("flag1\n");
#endif
		send_byte(control, num, control->shared_ptr->node[num].to_send.token_flag);
		control->snd_state = TO; // move to next case
		// SIGNAL_SEM(control,CRIT);
		break;

	case TO:
// TEST
#ifdef DEBUG
		printf("flag2\n");
#endif
		send_byte(control, num, control->shared_ptr->node[num].to_send.to);
		control->snd_state = FROM; // move to next case
		break;

	case FROM:
#ifdef DEBUG
		printf("flag3\n");
#endif
		send_byte(control, num, control->shared_ptr->node[num].to_send.from);
		control->snd_state = LEN; // move to next case
		break;

	case LEN:
#ifdef DEBUG
		printf("flag4\n");
#endif
		sndlen = control->shared_ptr->node[num].to_send.length;
		sndpos = 0;
		send_byte(control, num, control->shared_ptr->node[num].to_send.length);
		control->snd_state = DATA; // move to next case
		break;

	case DATA:
#ifdef DEBUG
		printf("flag5\n");
#endif
		send_byte(control, num, control->shared_ptr->node[num].to_send.data[sndpos++]); // update and increment val
		if (sndpos == sndlen)
		{
			control->snd_state = DONE; // move to next case
		}
		else
		{
			control->snd_state = DATA; // reloop
		}
		break;

	case DONE:
#ifdef DEBUG
		printf("flag6\n");
#endif
		// reset all values and send the token to the next node
		sndpos = 0;
		sndlen = 0;
		control->shared_ptr->node[num].to_send.length = 0; //to indicate to stop calling snd_pkt
		control->shared_ptr->node[num].sent++;
		control->snd_state = TOKEN_FLAG;
		control->shared_ptr->node[num].to_send.token_flag = '1';
		send_byte(control, num, '1'); // push the token to the next node
		SIGNAL_SEM(control, TO_SEND(num));
		break;
	};
}

/*
 * Send a byte to the next node on the ring.
 */
void
	send_byte(control, num, byte) struct TokenRingData *control;
int num;
unsigned byte;
{
	WAIT_SEM(control, EMPTY((num + 1) % N_NODES));
	// WAIT_SEM(control, CRIT);
	control->shared_ptr->node[(num + 1) % N_NODES].data_xfer = byte;
#ifdef DEBUG
	fprintf(stderr, "Node %d: Sending byte=%c to Node %d\n", num, byte, (num + 1) % N_NODES);
#endif
	SIGNAL_SEM(control, FILLED((num + 1) % N_NODES));
}

/*
 * Receive a byte for this node.
 */
unsigned char
rcv_byte(control, num)
struct TokenRingData *control;
int num;
{
	unsigned char byte;

	WAIT_SEM(control, FILLED(num));
	byte = control->shared_ptr->node[num].data_xfer;
	SIGNAL_SEM(control, EMPTY(num));
// TEST
#ifdef DEBUG
	fprintf(stderr, "Node %d: Received byte=%c\n", num, byte);
#endif
	return byte;
}
