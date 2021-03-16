// sudo ip addr add 10.0.0.1/24 dev tun0
// sudo ip link set tun0 up

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/if.h>
#include <linux/if_tun.h>

#define DEV_NAME_SIZE 16
#define BUFFER_SIZE 10000

int tun_alloc(char* dev)
{
  struct ifreq ifr;

  int fd = open("/dev/net/tun", O_RDWR);
  if (fd < 0) {
    return fd;
  }

  memset(&ifr, 0, sizeof(ifr));

  ifr.ifr_flags = IFF_TUN | IFF_NO_PI;

  if (*dev == 0) {
    strncpy(ifr.ifr_name, dev, DEV_NAME_SIZE);
  }

  int err = ioctl(fd, TUNSETIFF, (void*)&ifr);
  if (err < 0) {
    close(fd);
    return err;
  }

  strcpy(dev, ifr.ifr_name);

  return fd;
}

int main(int argc, char** argv)
{
  char dev_name[DEV_NAME_SIZE] = {};
  int fd = tun_alloc(dev_name);

  if (fd >= 0) {
    printf("Device created: %s\n", dev_name);
  } else {
    printf("Device creation error\n");
  }

  char buffer[BUFFER_SIZE];
  unsigned char ip[4];
  while(1) {
    sleep(1);

    int nread = read(fd, buffer, sizeof(buffer));
    if (nread < 0) {
      perror("error!\n");
      break;
    }

    printf("Read %d bytes\n", nread);

    memcpy(ip, &buffer[12], 4);
    memcpy(&buffer[12], &buffer[16], 4);
    memcpy(&buffer[16], ip, 4);

    buffer[20] = 0;
    *((unsigned short *)&buffer[22]) += 8;
        
    nread = write(fd, buffer, nread);
 
    printf("Write %d bytes to tun/tap device\n", nread);
  }

  close(fd);

  return 0;
}
