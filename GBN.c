#include "../include/simulator.h"
#define NULL 0

const DEFAULT_ACK=111;
const TIMEOUT=30.0;
const Host_A=0;
const Host_B=1;

int B_next_seq_num = 0;
int A_next_seq_num=0;
int pkt_in_window=0;
struct pkt *packets;

struct node {
  struct msg message;
  struct node *next;
};

struct node *node_head = NULL;
struct node *node_end = NULL;

int A_window_first_packet = 0;
int last_transmitted_packet=0;


/* called from layer 5, passed the data to be sent to other side */
void A_output(message)
  struct msg message;
{
  struct node *n_node;
  struct node *p;
  struct msg *m = &message;
  struct node *n = malloc(sizeof(struct node));
  int i = 0;

  if(n == NULL) {
    printf("Insufficient memory in the buffer.");
    goto Label1;
  } else {
    n->next = NULL;
    while(i < 20) {
      n->message.data[i] = m->data[i];
      i++;
    }

    if(node_end == NULL) {
      node_head = n;
      node_end = n;
      goto Label1;
    } else {
      node_end->next = n;
      node_end = n;
    }
  }

  Label1: if(pkt_in_window == getwinsize()) {
    return;
  }
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

  int next_expected_packet = (last_transmitted_packet+1)%getwinsize();

  if(next_expected_packet==A_window_first_packet) {
    return;
  } else {
    int i = 0;
    if(pkt_in_window!=0) {
       last_transmitted_packet=(last_transmitted_packet+1)%getwinsize();
    }
  
    packets[last_transmitted_packet];
    
    while (i<20) {
      packets[last_transmitted_packet].payload[i] = n->message.data[i];
      i++;
    }

    packets[last_transmitted_packet].seqnum = A_next_seq_num;
    packets[last_transmitted_packet].acknum = DEFAULT_ACK;
    struct pkt *p1 = &packets[last_transmitted_packet];
    if(p1 == NULL) {
      return 0;
    } else {
    packets[last_transmitted_packet].checksum = calc_checksum(&packets[last_transmitted_packet]);
    }
    A_next_seq_num++;
    pkt_in_window++;
    tolayer3(Host_A, packets[last_transmitted_packet]);
    if(A_window_first_packet==last_transmitted_packet) {
      starttimer(Host_A, TIMEOUT);
    }
    free(n_node);
    return 0;
  }
}

/* called from layer 3, when a packet arrives for layer 4 */
void A_input(packet)
  struct pkt packet;
{
  
  if((&packet == NULL || (&packet != NULL && packet.checksum != calc_checksum(&packet))) || (packet.acknum != packets[A_window_first_packet].seqnum)) {
    return;
  } else {
    stoptimer(Host_A);
    packets[A_window_first_packet].seqnum=-1;
    pkt_in_window--;
    
    if(pkt_in_window==0)
    {
      struct node *p;
      struct node *n_node;
      int i = 0;
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
      Label2: while(n_node!=NULL)
      {  
        packets[last_transmitted_packet];
        while (i<20)
        {
          packets[last_transmitted_packet].payload[i] = n_node->message.data[i];
          i++;
        }
        free(n_node);

        packets[last_transmitted_packet].seqnum = A_next_seq_num;
        packets[last_transmitted_packet].acknum = DEFAULT_ACK;
        struct pkt *p1 = &packets[last_transmitted_packet];
        if(p1 == NULL) {
          return 0;
        } else {
        packets[last_transmitted_packet].checksum = calc_checksum(&packets[last_transmitted_packet]);
        }
        A_next_seq_num++;
        pkt_in_window++;
        tolayer3(Host_A, packets[last_transmitted_packet]);
        starttimer(Host_A, TIMEOUT);
      }
    } else {
      A_window_first_packet=(A_window_first_packet+1)%getwinsize();
      struct node *n_node;
      struct node *p;
      if(node_head == NULL) {
        n_node = NULL;
        goto Label3;
      }
      p = node_head;
      node_head = p->next;
      if(node_head == NULL) {
        node_end = NULL;
      }
      n_node = p;

      Label3: if(n_node!=NULL) {
        int i = 0;
        last_transmitted_packet=(last_transmitted_packet+1)%getwinsize();  
        packets[last_transmitted_packet];
        while (i<20)
        {
          packets[last_transmitted_packet].payload[i] = n_node->message.data[i];
          i++;
        }
        free(n_node);

        packets[last_transmitted_packet].seqnum = A_next_seq_num;
        packets[last_transmitted_packet].acknum = DEFAULT_ACK;
        struct pkt *p1 = &packets[last_transmitted_packet];
        if(p1 == NULL) {
          return 0;
        } else {
          packets[last_transmitted_packet].checksum = calc_checksum(&packets[last_transmitted_packet]);
        }
        A_next_seq_num++;
        pkt_in_window++;
        tolayer3(Host_A, packets[last_transmitted_packet]);
      }
    }

    if(A_window_first_packet != last_transmitted_packet || pkt_in_window==1)
    {
      starttimer(Host_A, TIMEOUT);
    }
  }
}

/* called when A's timer goes off */
void A_timerinterrupt()
{
  int i=A_window_first_packet;
  while(1) {
    if(i==last_transmitted_packet) {
      tolayer3(Host_A, packets[i]);
      break;
    }
    tolayer3(Host_A, packets[i]);
    i=(i+1)%getwinsize();
  }
  if(A_window_first_packet != last_transmitted_packet || pkt_in_window==1) {
    starttimer(Host_A, TIMEOUT);
  }
}  

void A_init()
{
  int Window_size=getwinsize();
  packets = malloc(sizeof(struct pkt) * Window_size);
}

void B_input(packet)
  struct pkt packet;
{
  if(&packet == NULL || (&packet != NULL && packet.checksum != calc_checksum(&packet))) {
    return;
  }
  
  if(packet.seqnum == B_next_seq_num) {
    ++B_next_seq_num;
    tolayer5(Host_B, packet.payload);
  }
  
  if(packet.seqnum < B_next_seq_num)
    {
      packet.acknum = packet.seqnum ;
      struct pkt *p1 = &packet;
      if(p1 == NULL) {
        return 0;
      } else {
        packet.checksum = calc_checksum(&packet);
      }
      tolayer3(Host_B, packet);
    }
  
}


void B_init()
{

}

int calc_checksum(struct pkt *p)
{
  int checksum = 0;
  int i = 0;
  while (i<20)
  {
    checksum += (unsigned char)p->payload[i];
    i++;
  }
  checksum += p->seqnum + p->acknum;
  return checksum;
}

