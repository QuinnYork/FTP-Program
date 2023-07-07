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

#define PORT 21
#define MAX 4096
#define ERROR -1
#define MAX_IP 255
#define MAX_IP_LEN 15

char* get_home_dir(void) {
  struct passwd *pwd;
  pwd = getpwuid(getuid());

  return pwd->pw_dir;
}

int ip_validation(char* ip_addr) {
  int v1 = 0, v2 = 0, v3 = 0, v4 = 0; // xxx.xxx.xxx.xxx (4 different values delimited by a period)
  int dot_count = 0, iter = 0;
  // IP values must be:
  // 1. 0<=V<=255
  // 2. have 4 total values (0 is allowed)
  while (ip_addr[iter] != '\0') {
    if (ip_addr[iter++] == '.')
      dot_count++; // count dots found
  }

  if (dot_count != 3)
    return ERROR;

  // place input into variables
  sscanf(ip_addr, "%d.%d.%d.%d", &v1, &v2, &v3, &v4);

  // check values placed into int variables
  if (v1 < 0 || v2 < 0 || v3 < 0 || v4 < 0 ||
      v1 > MAX_IP || v2 > MAX_IP || v3 > MAX_IP || v4 > MAX_IP)
    return ERROR;
  
  return 0;
}

void transfer(int serv_sock) {

  // init vars
  char buf[MAX];
  char* pwd = malloc(MAX);
  pwd = get_home_dir();

  int bytes;
  
  while (1) {

    bzero(buf, MAX);
    bytes = 0;
    FILE* fp_transfer;
    // while we have a connection, transfer files
    // provide a filepath (could change later with a directory GUI)
    // input will be printed but also loaded into a FILE* that will be opened
    // and read to the server. server could potentially open its own file
    // and add to it as it gets data (read func)
    printf("\n%s: \n\tEnter filepath to be transferred: ", pwd);
    fgets(buf, MAX, stdin);

    if (!strcmp(buf, "exit")) {
      printf("Terminating connection with server...\n");
      break; // break out of loop and close sockets
    }
    
    fp_transfer = fopen(buf, "r");

    if (fp_transfer == NULL) {
      printf("File does not exist.\n");
      continue;
    }

    // start writing data to server who is reading into a file descriptor
    // fgets or read?
    // keep reading
    bytes = MAX;
    printf("Transferring file...\n");
    while (fgets(buf, bytes, fp_transfer) != NULL) {  
      bytes = strlen(buf); 
      write(serv_sock, buf, bytes);
    }

    read(serv_sock, buf, MAX);  
    printf("Server response: %s\n", buf);
  }

}

void receive(int serv_sock)
{
  // init vars
  char buf[MAX];
  char* pwd = malloc(MAX);
  pwd = get_home_dir();

  FILE* out;

  int bytes;
  
  while (1) {

    bzero(buf, MAX);
    bytes = 0;
    // while we have a connection, transfer files
    // provide a filepath (could change later with a directory GUI)
    // input will be printed but also loaded into a FILE* that will be opened
    // and read to the server. server could potentially open its own file
    // and add to it as it gets data (read func)
    printf("\n%s: \n\tEnter filepath to be transferred (or exit): ", pwd);
    fgets(buf, MAX, stdin);

    if (!strcmp(buf, "exit")) {
      printf("Exiting back to main...\n");
      break; // break out of loop and choose t,r, or e
    }

    // get filename from buffer
    // initial alg: parse string and count how many '/''s there are
    // parse the buffer again and get the string after the last '/'

    // TODO: TEST THIS ALGORITHM
    // just a brute force for now
    // could do in one loop where I go until I reach null char and then
    // walk backwards until I reach a '/' char

    // could also start at buf[strlen(buf) - 1] and go until I reach a '/', then walk
    // back the other way, adding each char to filename
    int idx = (strlen(buf) - 1), count = 0, fwd_bwd = 0;
    char* filename;
    bzero(filename, strlen(buf));
    while (count >= 0)
    {
      if (buf[idx--] == '/') {
	fwd_bwd = 1;
	realloc(filename, (count + 1));
      } else if (!fwd_bwd)
	count++;
      else
      {
	filename[idx] = buf[idx];
	count--;
	idx++;
      }
    }
    
    out = fopen(filename, "w+");

    if (out == NULL)
    {
      printf("File could not be created. Please try again.\n");
      continue;
    }
    
  }
}

int main(int argc, char* argv[]) {
  // initialize variables
  // buffers, sockets, addrs, fds, etc.
  
  int sockfd;

  char serv_msg[MAX];
  char client_in[MAX];
  char ip_addr[MAX_IP_LEN];

  struct sockaddr_in server;

  bzero(serv_msg, MAX);
  bzero(client_in, MAX);
  bzero(ip_addr, MAX_IP_LEN);
  bzero(&server, sizeof(struct sockaddr_in));

  if (argc <= 1) {
    printf("Need an IP address as argument!\n");
    exit(0);
  }

  // put argument into ip_addr string
  strcpy(ip_addr, argv[1]);
  
  // vars are initialized
  // now create socket and attempt to connect
  // client side is much less complicated since we are asking to be accepted
  // the server has to be the one to decide if it wants to accept

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd == ERROR) {
    printf("Socket couldn't be created.\n");
    exit(1);
  }

  // INET, PORT 21, serv IP
  server.sin_family = AF_INET;
  server.sin_port = htons(PORT);
  server.sin_addr.s_addr = inet_addr(ip_addr);

  // socket has been written to. need to attempt connection to server
  if (connect(sockfd, (struct sockaddr*)&server, sizeof(server)) == -1)
  {
    printf("Couldn't connect to server. Exiting now...\n");
    exit(1);
  }

  // connected to server by this point. can call function that deals with FTP shenanigans
  // could have two functions inside of a while loop with a prompt beforehand asking
  // what kind of FTP functions it wants to use.
  // It can either a) transfer to server or b) get files from server
  // i.e. transfer(serv_sock) & receive(serv_sock)
  // one writes then receives a response; the other sends a file to get and reads data
  // into a fd
  printf("Connected!\n\nWould you like to (t)ransfer a file, (r)eceive a file, or (e)xit?: ");
  char command = getchar();
  while (command != 'e' || command != 'E')
  {
    if (command != 't' || command != 'T' || command != 'r' || command != 'R')
    {
      printf("\nError: please enter a 't' for transfers, 'r' for receiving, or 'e' for exiting.\n");
      command = getchar();
      continue;
    }
    else if (command == 't' || command == 'T')
      transfer(sockfd);
  //else
  //  receive(sockfd);
  }
  
  
  return 0;
  
}




