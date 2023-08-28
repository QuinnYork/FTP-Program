#include<stdio.h>
#include<sys/types.h>//socket
#include<sys/socket.h>//socket
#include<stdlib.h>//sizeof
#include<unistd.h>
#include<arpa/inet.h>
#include<string.h>
#include<ctype.h>//isdigit()
#include<fcntl.h>//open()
#include<dirent.h>
#include<sys/stat.h>//stat()
#include<grp.h>
#include<pwd.h>
#include<time.h>

#define MAX 4096
#define PORT 20

int main(int argc, char* argv[])
{
  int sockfd, clifd;
  char buf[MAX], pwd[MAX], instr[MAX];
  struct sockaddr_in serv, cli;

  socklen_t length;
  
  sockfd = socket(AF_INET, SOCK_STREAM, 0);

  if (sockfd == -1)
  {
    printf("Error creating socket. Try again.");
    exit(1);
  }

  bzero(&serv, sizeof(serv));
  serv.sin_family = AF_INET;
  serv.sin_addr.s_addr = htonl(INADDR_ANY);
  serv.sin_port = htons(PORT);

  if (bind(sockfd, (struct sockaddr*)&serv, sizeof(serv)) != 0)
  {
    printf("Error binding.\n");
    exit(1);
  }

  if (listen(sockfd, PORT) != 0)
  {
    printf("Error listening for client.\n");
    exit(1);
  }

  // while we have > 0 connections
  // TODO: add asynchronous later
  while (1)
  {
    length = sizeof(serv);

    clifd = accept(sockfd, (struct sockaddr*)&cli, &length);

    if (clifd == -1)
    {
      printf("Error with accept\n");
      exit(1);
    }

    // create thread, call acting function
    
  }

  
}
