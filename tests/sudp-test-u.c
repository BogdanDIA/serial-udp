/*
 * Copyright BogdanDIA@github.com
 */
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <termios.h>
#include <getopt.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <signal.h>
#include <poll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/time.h>
#include <stdint.h>
#include <endian.h>

int debug_data = 0;
int debug_flow = 0;

// GCC style
#define PRINTF_LOG(fmt, ...) \
            do { if (debug_flow) printf(fmt, ##__VA_ARGS__); } while (0)

static void usage(void)
{
  printf("sudp-test-u - Utility for testing UDP RTT\n");
  printf("Usage:\n"
    "\tsudp-test-u -s <server> [-p <port>] [-L <plen>] [-N] [-F] [-D] [-h]\n");
}

static struct option main_options[] = {
  { "server", 1, 0, 's' },
  { "port", 0, 0, 'p' },
  { "plen", 0, 0, 'L' },
  { "nflow", 0, 0, 'N' },
  { "fdebug", 0, 0, 'F' },
  { "ddebug", 0, 0, 'D' },
  { "help", 0, 0, 'h' },
  { 0, 0, 0, 0 }
};

int main(int argc, char *argv[])
{
  char p_hostname[50]; strcpy(p_hostname, "127.0.0.1");
  unsigned long p_portno = 3333;
  unsigned int p_param_length = 128;
  int p_debug_flow = 0;
  int p_debug_data = 0;
  int p_no_rtscts = 0;

  int opt;

  while ((opt=getopt_long(argc, argv, "+s:p:L:NFDh", main_options, NULL)) != -1) {
    switch (opt) {
      case 's':
        strcpy(p_hostname, optarg);
        break;
      case 'p':
        ; // satisfy the std
        char * pPortno;
        p_portno = strtoul(optarg, &pPortno, 10);
        if(p_portno == 0)
        {
          printf("wrong portno format: %s\n", optarg); 
          exit(-1);
        }
        break;
      case 'L':
        ; // satisfy the std
        char * pLen;
        p_param_length = strtoul(optarg, &pLen, 10);
        if(p_param_length == 0)
        {
          printf("wrong param_length format: %s\n", optarg); 
          exit(-1);
        }
        break;
      case 'F':
        p_debug_flow = 1;
        break;
      case 'D':
        p_debug_data = 1;
        break;
      case 'N':
        p_no_rtscts = 1;
        break;
      case 'h':
      default:
        usage();
        exit(0);
    }
  }

  argc -= optind;
  argv += optind;
  optind = 0;

  // hostname is mandatory
  if (p_hostname[0] == 0)
  {
    printf("no devicename was provided, exitting...\n");
    exit(-1);
  }

  // show byte order
  if (BYTE_ORDER == BIG_ENDIAN)
    printf("platform is BIG_ENDIAN\n");
  else if(BYTE_ORDER == LITTLE_ENDIAN)
    printf("platform is LITTLE_ENDIAN\n");
  
  printf("server: %s\n", p_hostname); 
  printf("port: %lu\n", p_portno); 
  printf("param_length: %u\n", p_param_length); 
  printf("fdebug: %i\n", p_debug_flow); 
  printf("ddebug: %i\n", p_debug_data); 
  printf("nrtscts: %i\n", p_no_rtscts); 

#define BUFSIZE 2048 
  unsigned char buf_tx[BUFSIZE];
  unsigned char buf_rx[BUFSIZE];

  int sockfd;
  struct sockaddr_in serveraddr;
  socklen_t serverlen;
  struct hostent *server;
  
  // create the socket
  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd < 0)
    PRINTF_LOG("error opening socket\n");

  // gethostbyname: get the server's DNS entry
  server = gethostbyname(p_hostname);
  if (server == NULL) {
      fprintf(stdout,"error, no such host as %s\n", p_hostname);
      return (-1);
  }

  // build the server's Internet address
  bzero((char *) &serveraddr, sizeof(serveraddr));
  serveraddr.sin_family = AF_INET;
  bcopy((char *)server->h_addr,
  (char *)&serveraddr.sin_addr.s_addr, server->h_length);
  serveraddr.sin_port = htons(p_portno);
  serverlen = sizeof(serveraddr);

  // fork - child will handle UDP receive
  pid_t child_pid;
  child_pid = fork();

  if (child_pid < 0) {
    perror("Fork failed");
    return (-1);
  } else if (child_pid == 0) {
    // In child process
    PRINTF_LOG("Child process: UDP reader\n");

    while(1)
    {
      usleep(1000000);
    }
  } else {
    // In parent process
    PRINTF_LOG("Parent process: Child process ID is %d\n", child_pid);

    // for timings measuring
    struct timeval tx_tv, rx_tv;

    // TX vars
    int tx_size = 0;

    // RX vars
    float speed_kbps = 0.0;

    while(1)
    {
      // prepare TX message (header+data)
      // 0x0A + params_length + params

      // TX size
      int param_length = 512;
      tx_size = 3 + param_length;

      // prepare TX header
      buf_tx[0] = 0x0a;
      // send in LE format
      uint16_t param_length_le = htole16(param_length);
      buf_tx[1] = (unsigned char)(param_length_le & 0xff);
      buf_tx[2] = (unsigned char)((param_length_le >> 8) & 0xff);

      // prepare TX data
      for (int i = 0; i < (param_length); i++)
        buf_tx[i + 3] = i & 0xff;


      gettimeofday(&tx_tv, NULL);

      /* send the message to the server */
      int n = sendto(sockfd, buf_tx, tx_size, 0, (struct sockaddr *)&serveraddr, serverlen);
      if (n < 0)
        PRINTF_LOG("error in sendto\n");
      else
        PRINTF_LOG("end frame - wrote %i bytes to %s\n", tx_size, "udpport");

      // print the server's reply
      n = recvfrom(sockfd, buf_rx, BUFSIZE, 0, (struct sockaddr *)&serveraddr, &serverlen);
      if (n < 0)
        printf("error in recvfrom\n");

      if (n > 0)
      {
        PRINTF_LOG("\nreceived %i bytes from %s\n", n, "udpport");

        if (debug_data)
        {
          for (int i = 0; i < n; i++)
            PRINTF_LOG("%02x ", (unsigned char)buf_rx[i]);
          PRINTF_LOG("\n");
        }
      }

      gettimeofday(&rx_tv, NULL);
      long int time_tx = (long int)tx_tv.tv_sec * 1000000L + (long int)tx_tv.tv_usec;
      long int time_rx = (long int)rx_tv.tv_sec * 1000000L + (long int)rx_tv.tv_usec;
      long int time_rx_tx = time_rx - time_tx;
      speed_kbps = (float)(1000 * 8 * n / time_rx_tx);
      printf("end frame: rx-tx: %li, n: %i, speed_kbps: %f\n", time_rx_tx, n, speed_kbps);

      //usleep(100000);
    }

    // Wait for the child process to complete
    int status;
    waitpid(child_pid, &status, 0);
    if (WIFEXITED(status)) {
      //PRINTF_LOG("Parent process: Child exited with status %d\n", WEXITSTATUS(status));
      printf("Parent process: Child exited with status %d\n", WEXITSTATUS(status));
    }
  }

  return (0);
}
