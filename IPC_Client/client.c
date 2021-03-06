#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <pthread.h>

//resets buffer
void reset_buff(char *buffer){
  memset(buffer, 0x0, sizeof(buffer));
}//dub

struct msgbuf {
   long mType;
   char mText[1024];
};

// Reset buffer then Copy text content to buffer
// Send N bytes of BUF to socket FD. Prints send data or error msg if there is a error.
void send_safe(int sock, char *buffer, int buff_len, char *text){

  reset_buff(buffer); // Reset buffer

  strcpy(buffer, text); // Copy text content to buffer
  
  int send_success = send(sock, buffer, buff_len, 0); // Send N bytes of BUF to socket FD. Returns the number sent or -1.

  if (send_success < 0){
    perror("[-]send error: ");
    exit(1);
  }
  else{
    printf("Client: %s\n \n", buffer);
  }
}

// Reset buffer
// Read N bytes into BUF from socket FD. Prints received data or error msg if there is a error.
void recv_safe(int sock, char *buffer, int buff_len){

  reset_buff(buffer); // Reset buffer
  
  int recv_success = recv(sock, buffer, buff_len, 0); // Read N bytes into BUF from socket FD. Returns the number read or -1 for errors.

  if (recv_success < 0){
    perror("[-]recv error");
    exit(1);
  }

  else if (buffer[0] == 0x0)
  {
  }

  else{
   printf("Server: %s\n \n", buffer); 
  }
}

/*
Open a connection on socket FD to peer at ADDR (which LEN bytes long).
Prints confirmation on success or error msg for errors. 
*/
void connect_safe(int sock, struct sockaddr_in addr, int addr_len){

  int connect_success = connect(sock, (struct sockaddr*)&addr, addr_len);
  
  if (connect_success < 0){
    perror("[-]connect error");
    exit(1);
  }
  else{
    printf("[+]Connected to the server.\n \n");
  }
}

int main(){

  char *ip = "127.0.0.1";
  int port = 8888;

  int sock;
  struct sockaddr_in addr;
  socklen_t addr_size;
  char buffer[1024];  
  int n;

  int qId;
  key_t key;
  struct msgbuf msg, buf;
  struct msqid_ds msgCtlBuf;

  if ( ( key = ftok( "/tmp", 'C' ) ) == -1 ) {
    perror( "server: ftok failed:" );
    exit( 1 );
  }

  printf( "server: System V IPC key = %u\n", key );

  if ( ( qId = msgget( key, IPC_CREAT | 0666 ) ) == -1 ) {
    perror( "server: Failed to create message queue:" );
    exit( 2 );
  }

  printf( "server: Message queue id = %u\n", qId );

  sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0){
    perror("[-]Socket error");
    exit(1);
  }
  else{
    printf("[+]TCP client socket created.\n \n");
  }

  memset(&addr, '\0', sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = port;
  addr.sin_addr.s_addr = inet_addr(ip);

  connect_safe(sock, addr, sizeof(addr));

  //send_safe(sock, buffer, sizeof(buffer), "1");

  //recv_safe(sock, buffer, sizeof(buffer));

  strcpy( msg.mText, "1" );
  msg.mType = 1;

  if ( msgsnd( qId, &msg, sizeof msg.mText, 0 ) == -1 ) {
    perror( "Client: msgsnd failed:" );
    exit( 3 );
  }

  printf( "Client: Message received = %s\n", msg.mText );

  int connected = 1;

  while(connected){

    recv_safe(sock, buffer, sizeof(buffer));

    sleep(1);

    if (atoi(buffer) >= 20 || atoi(buffer) == -1){
      connected = 0;
      
      //send_safe(sock, buffer, sizeof(buffer), "-1");

      strcpy( msg.mText, "-1" );
      msg.mType = 1;

      if ( msgsnd( qId, &msg, sizeof msg.mText, 0 ) == -1 ) {
        perror( "Client: msgsnd failed:" );
        exit( 3 );
      }

      printf( "Client: Message received = %s\n", msg.mText );

      if ( msgctl( qId, IPC_RMID, &msgCtlBuf ) == -1 ) {
        perror( "server: msgctl failed:" );
        exit( 4 );
      }

      printf( "Client: Message queue removed OK\n" );

    }

    else if ( atoi(buffer) > 0)
    {
      int answer = atoi(buffer) + 1;

      char str[80];

      sprintf(str, "%d", answer);

      //send_safe(sock, buffer, sizeof(buffer), str);

      strcpy( msg.mText, str );
      msg.mType = 1;

      if ( msgsnd( qId, &msg, sizeof msg.mText, 0 ) == -1 ) {
        perror( "Client: msgsnd failed:" );
        exit( 3 );
      }

      printf( "Client: Message received = %s\n", msg.mText );

    }

    else{
      printf("Failed");
    }
    
  }

  close(sock);
  printf("Sock closed.\n \n");
  printf("[+]Client Closed.\n\n");

  return 0;

}