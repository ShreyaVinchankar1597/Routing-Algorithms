#include "../include/simulator.h"
#define NULL 0

const INTERVAL=1.0;
const DEFAULT_ACK=111;
const TIMEOUT=20.0;
const Host_A=0;
const Host_B=1;
const BUFFER_MSG=1000;
 
struct node {
  struct msg message;
  struct node *next;
};
struct node *node_head = NULL;
struct node *node_end = NULL;
/*end*/
/* ******************************************************************
 ALTERNATING BIT AND GO-BACK-N NETWORK EMULATOR: VERSION 1.1  J.F.Kurose
   This code should be used for PA2, unidirectional data transfer
   protocols (from A to B). Network properties:
   - one way network delay averages five time units (longer if there
     are other messages in the channel for GBN), but can be larger
   - packets can be corrupted (either the header or the data portion)
     or lost, according to user-defined probabilities
   - packets will be delivered in the order in which they were sent
     (although some can be lost).
**********************************************************************/
 
/********* STUDENTS WRITE THE NEXT SEVEN ROUTINES *********/
 
/* called from layer 5, passed the data to be sent to other side */
struct packets_window *A_num_packets;
struct packets_window *B_num_packets;
 
int packets_in_window=0;
 
int A_window_first_packet = 0;
int A_window_last_packet = 0;
 
int B_window_first_packet = 0;
int B_window_last_packet = 0;
 
struct packets_window
{
  struct pkt packet_item;
  int ack;
  int timeout;
};
 
//sequence numbers
int A_seq_num = 0;
int B_seq_num = 0;
 
int temp=0; /////////////////////////////// Change this variable
float current_time=0;
int start_timer=0;
 
void A_output(message)
  struct msg message;
{
  struct node *n_node;
  struct node *p;
  struct msg *m = &message;
  struct node *n = malloc(sizeof(struct node));
  int i = 0;
  int j = 0;
 
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
 
  Label1: if(packets_in_window == getwinsize())
  {
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
  if(((A_window_last_packet+1)%getwinsize())==A_window_first_packet) {
    return;
  } else {
    if(packets_in_window!=0) {
      A_window_last_packet=(A_window_last_packet+1)%getwinsize();
    }
  }
  A_num_packets[A_window_last_packet];//the selected packet of the window
  while (j<20) {
    A_num_packets[A_window_last_packet].packet_item.payload[j] = n_node->message.data[j];
    j++;
  }
   
  free(n_node);
   
  A_num_packets[A_window_last_packet].packet_item.seqnum = A_seq_num;
  A_num_packets[A_window_last_packet].packet_item.acknum = DEFAULT_ACK;
  struct pkt *p1 = &A_num_packets[A_window_last_packet].packet_item;
  if(p1 == NULL) {
    return 0;
  } else {
    A_num_packets[A_window_last_packet].packet_item.checksum = calc_checksum(&A_num_packets[A_window_last_packet].packet_item);    
  }
    A_seq_num++;
    A_num_packets[A_window_last_packet].timeout=current_time+TIMEOUT;
    A_num_packets[A_window_last_packet].ack=0;
    packets_in_window++;
    tolayer3(Host_A, A_num_packets[A_window_last_packet].packet_item);
    if(start_timer==0)
    {
      start_timer=1;
      starttimer(Host_A,INTERVAL);
    }
    return 0;
}
 
/* called from layer 3, when a packet arrives for layer 4 */
void A_input(packet)
  struct pkt packet;
{
  int i = 0;
  if(&packet == NULL || (&packet != NULL && packet.checksum != calc_checksum(&packet))) {
    return;
  }
  if(packet.acknum == A_num_packets[A_window_first_packet].packet_item.seqnum) {
    A_num_packets[A_window_first_packet].ack=1;
    packets_in_window--;
    if(packets_in_window==0) {
      A_window_first_packet=(A_window_first_packet+1)%getwinsize();
      A_window_last_packet=(A_window_last_packet+1)%getwinsize();
      if(node_head!=NULL) {
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
          A_num_packets[A_window_last_packet];
          while (i<20)
          {
            A_num_packets[A_window_last_packet].packet_item.payload[i] = n_node->message.data[i];
            i++;
          }
 
          free(n_node);
 
          A_num_packets[A_window_last_packet].packet_item.seqnum = A_seq_num;
          A_num_packets[A_window_last_packet].packet_item.acknum = DEFAULT_ACK;
          struct pkt *p1 = &A_num_packets[A_window_last_packet].packet_item;
          if(p1 == NULL) {
            return 0;
          } else {
            A_num_packets[A_window_last_packet].packet_item.checksum = calc_checksum(&A_num_packets[A_window_last_packet].packet_item);
          }
 
          A_seq_num++;
          packets_in_window++;
          A_num_packets[A_window_last_packet].ack = 0;
          A_num_packets[A_window_last_packet].timeout= current_time + TIMEOUT;
         
 
          tolayer3(Host_A, A_num_packets[A_window_last_packet].packet_item);
        }
      } else {
        start_timer=0;
        stoptimer(Host_A);
      }
    }
    else {
      int i = A_window_first_packet;
      int j = 0;
 
      while(1) {
        int temp=(i+1)%getwinsize();
        if(i == A_window_last_packet || A_num_packets[temp].ack!=1) {
          break;
        }
        packets_in_window--;
        i=(i+1)%getwinsize();
        if(i==A_window_last_packet) {
          A_window_last_packet=i;
        }
      }
 
      A_window_first_packet=(i+1)%getwinsize();
      if(packets_in_window==0) {
        A_window_last_packet=A_window_first_packet;
      }
      struct node *n_node;
      struct node *p;
      if(node_head == NULL) {
        n_node = NULL;
        goto Label4;
      }
      p = node_head;
      node_head = p->next;
      if(node_head == NULL) {
        node_end = NULL;
      }
      n_node = p;
       
      Label4: if(n_node!=NULL) {  
        A_num_packets[A_window_last_packet];
        while (j<20) {
          A_num_packets[A_window_last_packet].packet_item.payload[j] = n_node->message.data[j];
          j++;
        }
       
        free(n_node);
 
        A_num_packets[A_window_last_packet].packet_item.seqnum = A_seq_num;
        A_num_packets[A_window_last_packet].packet_item.acknum = DEFAULT_ACK;
        struct pkt *p1 = &A_num_packets[A_window_last_packet].packet_item;
        if(p1 == NULL) {
          return 0;
        } else {
          A_num_packets[A_window_last_packet].packet_item.checksum = calc_checksum(&A_num_packets[A_window_last_packet].packet_item);
        }
        A_seq_num++;
        packets_in_window++;
        A_num_packets[A_window_last_packet].ack = 0;
        A_num_packets[A_window_last_packet].timeout = current_time + TIMEOUT;
       
        tolayer3(Host_A, A_num_packets[A_window_last_packet].packet_item);
      }
    }
  } else if (packet.acknum > A_num_packets[A_window_first_packet].packet_item.seqnum ) {
    int i=A_window_first_packet;
    while(1) {
      temp=(i+1)%getwinsize();
      if(i==A_window_last_packet) {
        break;
      }
      if(packet.acknum == A_num_packets[temp].packet_item.seqnum){
        A_num_packets[temp].ack=1;
        break;
      }
      i=(i+1)%getwinsize();
    }
  }
}
 
/* called when A's timer goes off */
void A_timerinterrupt()
{
 
  current_time=current_time+INTERVAL;
  if(packets_in_window != 0)
  {
    int i=A_window_first_packet;
    while(1) {
      if(i==A_window_last_packet) {
        break;
      }
      if(A_num_packets[i].ack==0&& A_num_packets[i].timeout<current_time)
      {
        A_num_packets[i].timeout=current_time+TIMEOUT;
        tolayer3(Host_A, A_num_packets[i].packet_item);
      }
      i=(i+1)%getwinsize();
    }

     if(A_num_packets[i].ack==0 && A_num_packets[i].timeout<current_time) {
        A_num_packets[i].timeout = current_time+TIMEOUT;
        tolayer3(Host_A, A_num_packets[A_window_first_packet].packet_item);
      }
  }
  starttimer(Host_A, INTERVAL);
}  

void A_init()
{
  int Window_size=getwinsize();
  int i = 0;
  A_num_packets= malloc(sizeof(struct packets_window) * Window_size);
  while(i<Window_size) {
    A_num_packets[i].ack==0;
    i++;
  }
  start_timer=1;
  starttimer(Host_A, INTERVAL);
}


void B_input(packet)
  struct pkt packet;
{
  if(&packet == NULL || (&packet != NULL && packet.checksum != calc_checksum(&packet))) {
    return;
  }
 
  if(packet.seqnum == B_seq_num) {
    B_seq_num=B_seq_num+1;
    tolayer5(Host_B, packet.payload);
    packet.acknum = B_seq_num-1; 
    struct pkt *p1 = &packet;
    if(p1 == NULL) {
      return 0;
    } else {
      packet.checksum = calc_checksum(&packet);
    }
    tolayer3(Host_B, packet);
   
    B_num_packets[B_window_first_packet].timeout=(B_seq_num)+getwinsize()-1;
 
    B_window_first_packet=(B_window_first_packet+1)%getwinsize();

    while(1) {
      if(B_num_packets[B_window_first_packet].packet_item.seqnum != B_seq_num) {
        break;
      }
      tolayer5(Host_B, B_num_packets[B_window_first_packet].packet_item.payload);
      B_seq_num++;
      B_num_packets[B_window_first_packet].timeout=(B_seq_num)+getwinsize()-1;
      B_window_first_packet=(B_window_first_packet+1)%getwinsize();
    }

  } else {
    int m = 0;
    if(packet.seqnum>B_seq_num) {
      if(packet.seqnum <= B_seq_num+getwinsize()) {
        while(m<getwinsize()) {
          if(B_num_packets[m].timeout==packet.seqnum)
          {
            B_num_packets[m].packet_item=packet;
            packet.acknum = packet.seqnum ;
            struct pkt *p1 = &packet;
            if(p1 == NULL) {
              return 0;
            } else {
            packet.checksum = calc_checksum(&packet);
            }
            tolayer3(Host_B, packet);
            break;
          }
          m++;
        }
      }
    }
    else {
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
}
 
void B_init()
{
  int Window_size = getwinsize();
  int i = 0;
  B_num_packets= malloc(sizeof(struct packets_window) * Window_size);
  while (i<Window_size) {
    B_num_packets[i].timeout = i;
    i++;
  }
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

