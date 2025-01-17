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

long int min_time_tx_rx = 0xffffffff;
long int max_time_tx_rx = 0;
long int avg_time_tx_rx = 0;

float min_speed_kbps = 1000000.0;
float max_speed_kbps = 0.0;
float avg_speed_kbps = 0.0;

int debug_flow = 0;

// GCC style
#define PRINTF_LOG(fmt, ...) \
            do { if (debug_flow) printf(fmt, ##__VA_ARGS__); } while (0)


int set_interface_attribs (int fd, int speed, int parity)
{
  struct termios tty;
  if (tcgetattr (fd, &tty) != 0)
  {
    PRINTF_LOG("error %d from tcgetattr\n", errno);
    return (-1);
  }

  cfmakeraw(&tty);
  cfsetospeed (&tty, speed);
  cfsetispeed (&tty, speed);

  // control flags
  tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     // 8-bit chars
  tty.c_cflag |= (CLOCAL | CREAD); // local control and read enabled
  tty.c_cflag |= CRTSCTS; // set HW clow control by default
  tty.c_cflag &= ~(HUPCL);  // disable hung-up
  tty.c_cflag &= ~(PARENB | PARODD);      // shut off parity
  tty.c_cflag |= parity;
  tty.c_cflag &= ~CSTOPB; // no stop bits
  
  // control chars
  tty.c_cc[VMIN]  = 1; // read doesn't block
  tty.c_cc[VTIME] = 0; // 0.5 seconds read timeout

  tty.c_cc[VINTR] = 0;
  tty.c_cc[VQUIT] = 0;
  tty.c_cc[VERASE] = 0;
  tty.c_cc[VKILL] = 0;
  tty.c_cc[VEOF] = 0;
  tty.c_cc[VEOL] = 0;
  tty.c_cc[VEOL2] = 0;
  tty.c_cc[VSWTC] = 0;
  tty.c_cc[VSTART] = 0;
  tty.c_cc[VSTOP] = 0;
  tty.c_cc[VSUSP] = 0;
  tty.c_cc[VREPRINT] = 0;
  tty.c_cc[VWERASE] = 0;
  tty.c_cc[VLNEXT] = 0;
  tty.c_cc[VDISCARD] = 0;

  // input flags
  tty.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl

  // local flags
  tty.c_lflag &= ~(HUPCL | ECHO | ECHOE | ECHOK | ECHOCTL | ECHOKE);
  
  if (tcsetattr (fd, TCSANOW, &tty) != 0)
  {
    PRINTF_LOG("error %d from tcsetattr\n", errno);
    return (-1);
  }
  return 0;
}

void set_serial_params (int fd, int should_block, int timeout, int no_hw_flowcontrol)
{
  struct termios tty;
  memset (&tty, 0, sizeof tty);
  if (tcgetattr (fd, &tty) != 0)
  {
    PRINTF_LOG("error %d from tcgetattr\n", errno);
    return;
  }

  tty.c_cc[VMIN]  = should_block ? 1 : 0;
  tty.c_cc[VTIME] = timeout;

  if (no_hw_flowcontrol)
    tty.c_cflag &= ~(CRTSCTS); //disable HW flow control
  else
    tty.c_cflag |= CRTSCTS; //enable HW flow control

  if (tcsetattr (fd, TCSANOW, &tty) != 0)
    PRINTF_LOG("error %d setting term attributes\n", errno);
}

static void usage(void)
{
  printf("sudp-test-a - Utility for testing RTT speed of sudp-forwarder\n\n");
  printf("Usage:\n"
    "\tsudp-test-a -d <dev> [-b <baud> ] [-L <plen>] [-N] [-F] [-D] [-M <nmesg>] [-h]\n");
}

static struct option main_options[] = {
  { "device", 1, 0, 'd' },
  { "baud", 1, 0, 'b' },
  { "plen", 0, 0, 'L' },
  { "nflow", 0, 0, 'N' },
  { "fdebug", 0, 0, 'F' },
  { "ddebug", 0, 0, 'D' },
  { "mmessages", 0, 0, 'M' },
  { "help", 0, 0, 'h' },
  { 0, 0, 0, 0 }
};

#define DEVICENAME_LEN_MAX 256

int main(int argc, char *argv[])
{
  char p_devicename[DEVICENAME_LEN_MAX]; p_devicename[0] = 0;
  unsigned int p_baud_i = 921600;
  unsigned int p_baud = B921600;
  unsigned int p_param_length = 128;
  int p_debug_flow = 0;
  int p_debug_data = 0;
  int p_no_rtscts = 0;
  int p_max_messages = 10;

  int opt;

  while ((opt=getopt_long(argc, argv, "+d:b:L:NFDM:h", main_options, NULL)) != -1) {
    char *pTmp;

    switch (opt) {
      case 'd':
        strncpy(p_devicename, optarg, DEVICENAME_LEN_MAX);
        break;
      case 'b':
        p_baud_i  = strtoul(optarg, &pTmp, 10);
        if(p_baud_i == 0)
        {
          fprintf(stderr, "wrong baud format: %s\n", optarg); 
          exit(-1);
        }
        switch (p_baud_i)
        {
          case 4800:
            p_baud = B4800;
            break;
          case 9600:
            p_baud = B4800;
            break;
          case 19200:
            p_baud = B4800;
            break;
          case 38400:
            p_baud = B4800;
            break;
          case 57600:
            p_baud = B4800;
            break;
          case 115200:
            p_baud = B115200;
            break;
          case 230400:
            p_baud = B4800;
            break;
          case 500000:
            p_baud = B4800;
            break;
          case 921600:
            p_baud = B921600;
            break;
          case 1000000:
            p_baud = B1000000;
            break;
          case 1152000:
            p_baud = B1152000;
            break;
          default:
            fprintf(stderr, "wrong baud rate: %i\n", p_baud_i); 
        }
        break;
      case 'L':
        p_param_length = strtoul(optarg, &pTmp, 10);
        if(p_param_length == 0)
        {
          fprintf(stderr, "wrong param_length format: %s\n", optarg); 
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
      case 'M':
        p_max_messages = strtoul(optarg, &pTmp, 10);
        if(p_max_messages == 0)
        {
          fprintf(stderr, "wrong max messages format: %s\n", optarg); 
          exit(-1);
        }
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

  // devicename is mandatory
  if (p_devicename[0] == 0)
  {
    fprintf(stderr, "no devicename was provided, exitting...\n");
    exit(-1);
  }

  // check if max messages is positive
  if (p_max_messages <= 0)
  {
    fprintf(stderr, "wrong max messages value: %i\n", p_max_messages);
    exit(-1);
  }

  // show byte order
  if (BYTE_ORDER == BIG_ENDIAN)
    printf("platform is BIG_ENDIAN\n");
  else if(BYTE_ORDER == LITTLE_ENDIAN)
    printf("platform is LITTLE_ENDIAN\n");

  // enable debug flow when debug data is enabled
  if (p_debug_data)
    p_debug_flow = 1;
  debug_flow = p_debug_flow;

  printf("device: %s\n", p_devicename); 
  printf("baud: %i\n", p_baud_i); 
  printf("param_length: %u\n", p_param_length); 
  printf("fdebug: %i\n", p_debug_flow); 
  printf("ddebug: %i\n", p_debug_data); 
  printf("nrtscts: %i\n", p_no_rtscts); 
  printf("mmessages: %i\n", p_max_messages); 

  int fd = open(p_devicename, O_RDWR | O_NOCTTY | O_SYNC);
  if (fd < 0)
  {
    fprintf(stderr, "error %d opening %s: %s\n", errno, p_devicename, strerror (errno));
    return (fd);
  }
  set_interface_attribs(fd, p_baud, 0);
  set_serial_params(fd, 1, 0, p_no_rtscts); // set blocking, no timeout, HW flow control enable/disable

#define BUFSIZE 2048 
  unsigned char buf_tx[BUFSIZE];
  unsigned char buf_rx[BUFSIZE];

  {
    // timing measuring vars
    struct timeval tx_tv, rx_tv, before_tv, after_tv, end_tv;

    // TX vars
    int tx_size = 0;
    int tx_n = 0;
    int tx_total_bytes = 0;

    // RX vars
    int nread = 0;
    int start_frame = 1;
    int total_bytes = 0;
    int packet_size = 0;
    long int total_us = 0;
    float speed_kbps = 0.0;

    for (int i = 0; i < p_max_messages; i++)
    {
      // prepare TX message (header+data)
      // 0x0A + params_length + params

      // TX size
      tx_size = 3 + p_param_length;

      // prepare TX header 
      buf_tx[0] = 0x0a;
      // send in LE format 
      uint16_t param_length_le = htole16(p_param_length); 
      buf_tx[1] = (unsigned char)(param_length_le & 0xff);
      buf_tx[2] = (unsigned char)((param_length_le >> 8) & 0xff);

      // prepare TX data 
      for (int i = 0; i < (p_param_length); i++)
        buf_tx[i + 3] = i & 0xff;

      tx_n = 0;
      tx_total_bytes = 0;

      // TX - loop until sending full data
      PRINTF_LOG("\nTX - sending test data\n");

      // this is timestamp of TX 
      gettimeofday(&tx_tv, NULL);

      do  
      {
        // print data
        if (p_debug_data)
        {
          for (int i = 0; i < tx_size; i++)
            printf("%02x ", buf_tx[i]);
          printf("\n");
        }
        
        // send data
        tx_n = write(fd, &buf_tx[tx_n], tx_size);
        if (tx_n > 0)
        {
          tx_total_bytes += tx_n;
          PRINTF_LOG("wrote tx_n: %i, tx_total_bytes: %i, tx_size: %i bytes to %s\n", tx_n, tx_total_bytes, tx_size, p_devicename);
        }
        else
        {
          // error
          fprintf(stderr, "write error from %s: %s\n", p_devicename, strerror(errno));
          exit (-1);
        }
      } while (tx_total_bytes < tx_size);

      
      // minimum value (cmd+len)
      nread = 3;
      total_bytes = 0;

      // RX - loop for receiving full reply
      PRINTF_LOG("RX - wating for reply\n");
      do
      {
        // read from serial port 
        gettimeofday(&before_tv, NULL);
        int n = read(fd, &buf_rx[total_bytes], nread);
        if (n > 0)
        { 
          if (start_frame == 1)
          {
            // this is partial frame timestamp after RX
            //gettimeofday(&before_tv, NULL);
            start_frame = 0;
          }

          gettimeofday(&after_tv, NULL);
          long int time_before = (long int)before_tv.tv_sec * 1000000L + (long int)before_tv.tv_usec;
          long int time_after = (long int)after_tv.tv_sec * 1000000L + (long int)after_tv.tv_usec;
          long int delta = (long int)(time_after - time_before);
          total_us += delta;
          before_tv = after_tv;

          PRINTF_LOG("partial frame - received %i bytes, delta: %li, total_us: %li\n", n, delta, total_us);

          if (p_debug_data)
          {
            for (int i = 0; i < n; i++)
              printf("%02x ", buf_rx[total_bytes + i]);
            printf("\n");
          }

          total_bytes += n;
          unsigned int length_start = 0;
          unsigned int length_size = 0;
          unsigned int length_val = 0;

          // determine type of packet
          switch(buf_rx[0])
          {
            // HCI TEST packet
            case 0x0A:
              PRINTF_LOG("partial frame - type HCI TEST: %02x\n", buf_rx[0]);
              length_start = 1;
              length_size = 2;
              if (total_bytes >= (length_start + length_size))
              {
                // BT is using LE
                uint16_t *plen = (uint16_t *)&buf_rx[length_start];
                length_val = (unsigned int)le16toh(*plen & 0xffff);

                nread = length_start + length_size + length_val - total_bytes;
                PRINTF_LOG("partial frame - param length: %i, nread: %i\n", length_val, nread);
              }
              //else
              //  continue;
              break;
            default:
              PRINTF_LOG("partial data - unknown HCI packet: %i\n", buf_rx[0]);
          }

          // check if we received whole packet
          packet_size = length_start + length_size + length_val;
          if (total_bytes >= packet_size)
          {
            gettimeofday(&end_tv, NULL);
            long int time_end = (long int)end_tv.tv_sec * 1000000L + (long int)end_tv.tv_usec;
            long int delta = (long int)(time_end - time_after);
            total_us += delta;
            speed_kbps = (float)(1000 * 8 * total_bytes / total_us);
            PRINTF_LOG("end frame: delta: %li, total_us: %li, total_bytes: %i, speed_kbps: %f\n", delta, total_us, total_bytes, speed_kbps);

            gettimeofday(&rx_tv, NULL);
            long int time_tx = (long int)tx_tv.tv_sec * 1000000L + (long int)tx_tv.tv_usec;
            long int time_rx = (long int)rx_tv.tv_sec * 1000000L + (long int)rx_tv.tv_usec;
            long int time_tx_rx = time_rx - time_tx;
            speed_kbps = (float)(1000 * 8 * total_bytes / time_tx_rx);
            PRINTF_LOG("end frame: tx-rx: %li, total_bytes: %i, speed_kbps: %f\n", time_tx_rx, total_bytes, speed_kbps);

            // save min time_tx_rx
            if (time_tx_rx < min_time_tx_rx)
              min_time_tx_rx = time_tx_rx; 
            // save max time_tx_rx
            if (time_tx_rx > max_time_tx_rx)
              max_time_tx_rx = time_tx_rx;
            // save min speed_kbps
            if (speed_kbps < min_speed_kbps)
              min_speed_kbps = speed_kbps;
            // save min speed_kbps
            if (speed_kbps > max_speed_kbps)
              max_speed_kbps = speed_kbps;
            // calculate avg time_tx_rx
            avg_time_tx_rx += time_tx_rx;
            // calculate avg speed_kbps
            avg_speed_kbps += speed_kbps;

            if (p_debug_data)
            {
              for (int i = 0; i < total_bytes; i++)
                printf("%02x ", buf_rx[i]);
              printf("\n");
            }

            /* print the message */
            if (p_debug_data)
            {
              for (int i = 0; i < total_bytes; i++)
                if(buf_rx[i] != buf_tx[i])
                  fprintf(stderr, "data mismatch: index: %i, TX: %02x, RX: %02x\n", i, buf_tx[i], buf_rx[i]); 
            }

            PRINTF_LOG("end frame - received %i bytes out of %i, from %s\n", total_bytes, packet_size, p_devicename);
            PRINTF_LOG("end frame: tx-rx: %li, total_bytes: %i, speed_kbps: %f\n", time_tx_rx, total_bytes, speed_kbps);

            // reset counters
            total_bytes = 0;
            total_us = 0;
            start_frame = 1;

            // read minimum bytes to include parameters length
            nread = 3;

            // exit RX loop
            break;
          }
        }
        else if(n == 0)
        {
          fprintf(stderr, "read 0 bytes, exiting...\n");
          exit (-1);
        }
        else
        {
          // error
          fprintf(stderr, "read error from %s: %s\n", p_devicename, strerror(errno));
          exit (-1);
        }
      } while(total_bytes < packet_size);
    }

    avg_time_tx_rx /= p_max_messages;
    avg_speed_kbps /= p_max_messages;
    int data_len = p_param_length;

    printf("result tx-rx: data_len: %i, min_time: %li,  max_time: %li, avg_time: %li, min_speed: %.1f, max_speed: %.1f, avg_speed: %.1f\n", \
      data_len, min_time_tx_rx, max_time_tx_rx, avg_time_tx_rx, min_speed_kbps, max_speed_kbps, avg_speed_kbps);

    // cleanup vars
    min_time_tx_rx = 0; max_time_tx_rx = 0; avg_time_tx_rx = 0;
    min_speed_kbps = 0.0; max_speed_kbps = 0.0; avg_speed_kbps = 0.0;
  }

  return (0);
}
