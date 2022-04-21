#include "../include/simulator.h"

#define NULL 0

const DEFAULT_ACK=111;
const TIMEOUT=20.0;
const Host_A=0;
const Host_B=1;
const BUFFER_MSG=1000;
 
int A_status=0;
int waiting_for_packet=0;
int waiting_for_ack=1;

struct node {
  struct msg message;
  struct node *next;
};
struct node *node_head = NULL;
struct node *node_end = NULL;

int A_seq_num = 0;
int B_seq_num = 0;

struct pkt current_packet;

void A_output(message)
  struct msg message;
{ 
  int checksum = 0;
  struct node *n_node;
  struct node *p;
  int i = 0;
  int j = 0;

  struct msg *m = &message;
  struct node *n = malloc(sizeof(struct node));

  if(n == NULL) {
    printf("Insufficient memory in the buffer.");
    goto Label1;
  } else {
    n->next = NULL;
    while(i < 20) {
      n->message.data[i] = m->data[i];
      i++;
    }

    if(node_end == NULL) 
    {
      node_head = n;
      node_end = n;
      goto Label1;
    } else {
      node_end->next = n;
      node_end = n;
    }
  }

  Label1: if(A_status != waiting_for_packet) {
    return;
  }

  A_status = waiting_for_ack;

  if(node_head == NULL) {
    n_node = NULL;
    goto Label2;
  }
  p = node_head;
  node_head = p->next;
  if(node_head == NULL) {
    node_end = NULL;
  }
  n_node = p;
  
  Label2: if(n_node == NULL) {
    return 0;
  }

  while(j < 20) {
    current_packet.payload[j] = n_node->message.data[j];
    j++;
  }

  free(n_node);
  current_packet.seqnum = A_seq_num;
  current_packet.acknum = DEFAULT_ACK;
  struct pkt *p1 = &current_packet;
  if(p1 == NULL) {
    return 0;
  } else {
    current_packet.checksum = calc_checksum(&current_packet);
  }
  tolayer3(Host_A, current_packet);

  starttimer(Host_A, TIMEOUT);
}

/* called from layer 3, when a packet arrives for layer 4 */
void A_input(packet)
  struct pkt packet;
{
  if(&packet == NULL || (&packet != NULL && packet.checksum != calc_checksum(&packet)) || (packet.acknum != A_seq_num))
  {
    printf("The acknowledgment is either corrupted or not expected");
    return;
  }

  A_seq_num = (A_seq_num + 1) % 2;
  A_status = waiting_for_packet;
}


void A_timerinterrupt()
{
  if(A_status == waiting_for_ack)
  {
    tolayer3(Host_A, current_packet);
    starttimer(Host_A, TIMEOUT);
  }
}  

void A_init()
{

}


void B_input(packet)
  struct pkt packet;
{
  if(&packet == NULL || (&packet != NULL && packet.checksum != calc_checksum(&packet))) {
    return;
  }

  if(packet.seqnum == B_seq_num) {
    B_seq_num = (B_seq_num + 1)%2;
    tolayer5(Host_B, packet.payload);
  }

  packet.acknum = packet.seqnum;
  struct pkt *p1 = &packet;
  if(p1 == NULL) {
    return 0;
  } else {
    packet.checksum = calc_checksum(&packet);
  }

  tolayer3(Host_B, packet);
}

void B_init()
{

}

int calc_checksum(struct pkt *p)
{
  int checksum = 0;
  int i = 0;
  while (i<20) {
    checksum += (unsigned char)p->payload[i];
    i++;
  }
  checksum += p->seqnum + p->acknum;
  return checksum;
}
