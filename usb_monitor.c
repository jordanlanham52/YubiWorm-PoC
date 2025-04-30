#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <fcntl.h>

static volatile sig_atomic_t running = 1;
static void  handle_signal(int sig) { running = 0; }

#define UEVENT_BUF_LEN 2048

static void parse_uevent(const char *buf, ssize_t len) {
    char action[16]    = {0};
    char subsystem[16] = {0};
    char devname[64]   = {0};

    for (ssize_t i = 0; i < len; i += strlen(buf+i) + 1) {
        const char *s = buf + i;
        if      (strncmp(s, "ACTION=",    7) == 0) strncpy(action,    s+7, sizeof(action)-1);
        else if (strncmp(s, "SUBSYSTEM=",10) == 0) strncpy(subsystem, s+10,sizeof(subsystem)-1);
        else if (strncmp(s, "DEVNAME=",   8) == 0) strncpy(devname,   s+8, sizeof(devname)-1);
    }

    if (strcmp(subsystem, "hidraw") != 0) return;

    char path[128];
    snprintf(path, sizeof(path), "/dev/%s", devname);

    if (strcmp(action, "add") == 0) {
        printf("[+] HID device added: %s\n", path);
        int fd = open(path, O_RDWR|O_NONBLOCK);
        if (fd < 0) {
            perror("    open");
            return;
        }
        
        unsigned char out[64] = { 0x00, 0x01, 0x02, 0x03 };
        ssize_t w = write(fd, out, sizeof(out));
        if (w < 0) perror("    write");
        else        printf("    wrote %zd bytes\n", w);

        unsigned char in[64];
        ssize_t r = read(fd, in, sizeof(in));
        if (r < 0) perror("    read");
        else {
            printf("    read %zd bytes:", r);
            for (ssize_t i = 0; i < r; i++) printf(" %02x", in[i]);
            printf("\n");
        }
        close(fd);
    }
    else if (strcmp(action, "remove") == 0) {
        printf("[-] HID device removed: %s\n", path);
    }
}

int main(void) {
    struct sockaddr_nl addr = { .nl_family = AF_NETLINK, .nl_groups = 1 };
    int sock = socket(AF_NETLINK, SOCK_RAW, NETLINK_KOBJECT_UEVENT);
    if (sock < 0) { perror("socket"); return 1; }
    if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind"); close(sock); return 1;
    }

    signal(SIGINT,  handle_signal);
    signal(SIGTERM, handle_signal);

    printf("Listening for HID (hidraw) hot-plug events…\n");
    while (running) {
        char buf[UEVENT_BUF_LEN];
        ssize_t len = recv(sock, buf, sizeof(buf), 0);
        if (len < 0) {
            if (errno == EINTR) continue;
            perror("recv"); break;
        }
        parse_uevent(buf, len);
    }

    close(sock);
    printf("Exiting.\n");
    return 0;
}
