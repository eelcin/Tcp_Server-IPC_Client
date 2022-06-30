#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <math.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <pthread.h>
#include <errno.h>

#define _OPEN_SYS_ITOA_EXT

int client_sock;

struct msgbuf {
   long mType;
   char mText[1024];
};

struct deneme
{
  int a;
  int b;
};

struct connection_thread_in
{
  int server_sock;
  struct sockaddr* client_addr;
  socklen_t addr_size;
};


//resets buffer
void reset_buff(char *buffer){
  memset(buffer, 0x0, sizeof(buffer));
}//dub

// Reset buffer then Copy text content to buffer
// Send N bytes of BUF to socket FD. Prints send data or error msg if there is a error.
void send_safe(int sock, char *buffer, int buff_len, char *text){

  reset_buff(buffer); // Reset buffer  

  strcpy(buffer, text); // Copy text content to buffer
  
  int send_success = send(sock, buffer, buff_len, 0); // Send N bytes of BUF to socket FD. Returns the number sent or -1.

  if (send_success < 0){
    perror("[-]send error: \n \n");
    exit(1);
  }
  else{
    printf("Server: %s\n \n", buffer);
  }
}

// Reset buffer
// Read N bytes into BUF from socket FD. Prints received data or error msg if there is a error.
void recv_safe(int sock, char *buffer, int buff_len){

  reset_buff(buffer); // Reset buffer
  
  int recv_success = recv(sock, buffer, buff_len, 0); // Read N bytes into BUF from socket FD. Returns the number read or -1 for errors.

  if (recv_success < 0){
    perror("[-]recv error\n \n");
    exit(1);
  }

  else if (buffer[0] == 0x0)
  {
  }

  else{
   printf("Client: %s\n \n", buffer); 
  }
}

void *wait_connection (void *input ){

  //int server_sock, struct sockaddr* client_addr, socklen_t addr_size
  while (1)
  {
    struct connection_thread_in *args = input;

    client_sock = accept(args->server_sock, args->client_addr, &args->addr_size);

    if (client_sock < 0){
      perror("[-]client_sock error \n \n");
      exit(1);
    }

    else{
      printf("[+]Client connected.\n \n");
    }
  }
}

void *myThreadFun(void *input){

  struct deneme *args = input;

  int c = args->a + args->b;

  printf("Sum: %d \n a: %d \n b: %d \n \n", c, args->a, args->b);
}

int main(){

  char *ip = "127.0.0.1";
  int port = 8080;

  int server_sock;
  struct sockaddr_in server_addr, client_addr;
  socklen_t addr_size;
  char buffer[1024];
  int n;

  int qId;
  key_t key;
  struct msgbuf msg, buf;
  struct msqid_ds msgCtlBuf;

  if ( ( key = ftok( "/tmp", 'C' ) ) == -1 ) {
    perror( "client: ftok failed:" );
    exit( 1 );
  }

  printf( "client: System V IPC key = %u\n", key );

  if ( ( qId = msgget( key, IPC_CREAT | 0666 ) ) == -1 ) {
    perror( "client: Failed to create message queue:" );
    exit( 2 );
  }

  printf( "client: Message queue id = %u\n", qId );

  server_sock = socket(AF_INET, SOCK_STREAM, 0);
  if (server_sock < 0){
    perror("[-]Socket error");
    exit(1);
  }
  printf("[+]TCP server socket created.\n \n");

  memset(&server_addr, 0x0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = port;
  server_addr.sin_addr.s_addr = inet_addr(ip);

  n = bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr));

  if (n < 0){
    perror("[-]Bind error");
    exit(1);
  }
  printf("[+]Bind to the port number: %d\n \n", port);

  listen(server_sock, 5);
  printf("Listening...\n \n");

  addr_size = sizeof(client_addr);
  
  printf("Hello : %d \n \n", addr_size);  

  pthread_t thread_id;

  struct connection_thread_in input;

  input.addr_size = addr_size;
  input.client_addr = (struct sockaddr*)&client_addr;
  input.server_sock = server_sock;

  pthread_create(&thread_id, NULL, wait_connection, (void *)&input);

  int connected = 1;

  while(connected){

    //recv_safe(client_sock, buffer, sizeof(buffer));

    if ( msgrcv( qId, &buf, sizeof msg.mText, 1, 0 ) == -1 )
      perror( "client: msgrcv failed:" );
    else
      printf( "client: = %s\n", buf.mText );

    sleep(1);

    if (atoi(buf.mText) == -1){

      connected = 0;
      close(client_sock);
      
      printf( "Server: Message queue removed OK\n" );

      printf("[+]Server Closed.\n\n");

      exit( 4 );
    }

    else if ( atoi(buf.mText) > 0)
    {
      int answer = atoi(buf.mText) + 1;

      char str[80];

      sprintf(str, "%d", answer);

      send_safe(client_sock, buffer, sizeof(buf.mText), str);

    }
    else{
      printf("Empty Round \n \n");
    }

  }

  pthread_join(thread_id, NULL);

  return 0;
}