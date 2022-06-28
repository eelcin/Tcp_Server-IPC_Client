#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <math.h>
#include <sys/types.h>
#include <sys/msg.h>

#define _OPEN_SYS_ITOA_EXT

struct msgbuf {
   long mType;
   char mText[1024];
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

int main(){

  char *ip = "127.0.0.1";
  int port = 9999;

  int server_sock, client_sock;
  struct sockaddr_in server_addr, client_addr;
  socklen_t addr_size;
  char buffer[1024];
  int n;

  int qId;
  key_t key;

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
  client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &addr_size);
  printf("[+]Client connected.\n \n");

  int connected = 1;

  while(connected){

    recv_safe(client_sock, buffer, sizeof(buffer));

    sleep(1);

    if (atoi(buffer) == -1){
      connected = 0;
      close(client_sock);
      printf("[+]Server Closed.\n\n");
    }

    else if ( atoi(buffer) > 0)
    {
      int answer = atoi(buffer) + 1;

      char str[80];

      sprintf(str, "%d", answer);

      send_safe(client_sock, buffer, sizeof(buffer), str);

    }
    else{
      printf("Empty Round \n \n");
    }

  }

  return 0;
}