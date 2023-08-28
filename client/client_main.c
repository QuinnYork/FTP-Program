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
#include<errno.h>

#define PORT 20
#define MAX 4096
#define ERROR -1
#define MAX_IP 255
#define MAX_IP_LEN 15

//void trim(char* input)
//{
//  int n = strlen(
//}

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

void get_instr(char* instr, char* ret)
{
  int idx = 0;
  char c = instr[idx];
  while (c != ' ' && c != '\n' && c != '\t' && c != '\r'
	 && c != '\0') // look for whitespace
  {
    ret[idx] = c;
    c = instr[++idx];
  }
  printf("%s\n", instr);
}

void reverse(char* str, int len)
{
  int end = len - 1;
  for (int i = 0; i < (len / 2); i++)
  {
    char temp = str[i];
    str[i] = str[end];
    str[end--] = temp;
  }
}

void get_filename_from_path(char* path, char* filename)
{
  // path == "/xxx/xxx/xxx/name"
  // get /name
  // start from strlen(path) and read chars into filename until
  // a '/' is encountered
  printf("Starting filepath: %s\n\n\n", path);
  int length = strlen(path) - 1, idx = 0;
  char c = path[length--];
  while (c != '/')
  {
    filename[idx++] = c;
    c = path[length--];
  }
  filename[idx++] = c;
  filename[idx] = '.';
  reverse(filename, strlen(filename)); // chars were copied in reversed
  printf("Filename: %s\n", filename);
}

void put(int serv_sock) {

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

void get(int serv_sock, char* command)
{
  // init vars
  char buf[MAX], filename[MAX];
  char* filepath = malloc(MAX);
  char* pwd = malloc(MAX);
  pwd = get_home_dir();
  FILE* out;
  int data_size;
  
  // need to tell server what command and arg was input
  // command: get /xxx/xxx/name
  if ((write(serv_sock, command, strlen(command))) == -1)
  {
    printf("Error writing to server...\n");
    return;
  }

  // move string past "get "
  // now looking at path
  // TODO: more args could be used
  // could have a memory leak here
  strcpy(filepath, command+4);

  bzero(filename, MAX);
  get_filename_from_path(filepath, filename);
  
  out = fopen(filename, "w+");
  if (out == NULL)
  {
    printf("Error creating file in client directory. Exiting...\n");
    return;
  }

  // Try to read server data stream into out file
  // If read fails or has certain string, an error occurred
  while ((data_size = read(serv_sock, buf, MAX)) != -1) // TODO
  {
    // done/error
    if (fputs(buf, out) < 0)
    {
      fclose(out);
      free(command), free(pwd);
      return;
    }
  }

  fclose(out);
  free(command), free(pwd);
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
    printf("Couldn't connect to server. Exiting now...\nErrorno: %d\n", errno);
    exit(1);
  }

  // TODO: Before I do this if statement I can do authentication (add later)
  
  // Big if statement below that deals with different cmd line instructions
  // mkdir: create new directory on server
  // cd: change directory
  // rm: remove file from server
  // get: receive file from server
  // put: send file to server
  // ls: list directory contents

  // TODO: add arguments later on

  // Print working directory for each new line
  // Call functions given the instruction (could be a switch statement)
  char* instr;
  char* pwd;
  char buf[10]; // used to hold instructions. relatively small strings so 10 bytes is used

  pwd = malloc(MAX);
  instr = malloc(MAX);
  
  strcpy(pwd, get_home_dir());
  
  while (1)
  {
    
    bzero(instr, MAX);
    bzero(buf, 10);
    
    pwd = (char*) realloc(pwd, strlen(pwd) + 1);
    printf("%s$ ", pwd);
    
    // skips over second time around
    // EDIT: changed to fgets
    fgets(instr, MAX, stdin);
    instr[strlen(instr)-1] = '\0';
    
    // get instruction (every char before a space)
    get_instr(instr, buf);

    if (!strcmp(buf, "cd")) 
      // cd(sockfd);
      continue;
    else if (!strcmp(buf, "mkdir"))
      // mkdir(sockfd);
      continue;
    else if (!strcmp(buf, "rm"))
      // rm(sockfd);
      continue;
    else if (!strcmp(buf, "get")) {
      if (strlen(instr) <= 4) {
	printf("Must enter a filepath when using get.\n");
	continue;
      }
      get(sockfd, instr);
    } else if (!strcmp(buf, "put"))
      put(sockfd);
    else if (!strcmp(buf, "ls"))
      // ls(sockfd);
      continue;
    else {
      printf("Error in getting new instr\n");
      exit(1);
    }
      
    
  }
  
  return 0;
  
}




