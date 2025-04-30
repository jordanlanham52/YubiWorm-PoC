#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/prctl.h>

#define UEVENT_BUFFER_SIZE 2048

static void daemonize(void) {
    pid_t pid;

    // 1st fork
    pid = fork();
    if (pid < 0) exit(1);
    if (pid > 0) exit(0);  

    if (setsid() < 0) exit(1);

    pid = fork();
    if (pid < 0) exit(1);
    if (pid > 0) exit(0);

    umask(0);

    chdir("/");

    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    prctl(PR_SET_NAME, "udevd", 0, 0, 0);
}

int main(void) {
    daemonize();

    int sock = socket(AF_NETLINK, SOCK_RAW, NETLINK_KOBJECT_UEVENT);
    if (sock < 0) exit(1);

    struct sockaddr_nl addr = {
        .nl_family = AF_NETLINK,
        .nl_pid    = getpid(),
        .nl_groups = -1
    };
    if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0)
        exit(1);

    char buf[UEVENT_BUFFER_SIZE];
    while (1) {
        int len = recv(sock, buf, sizeof(buf), 0);
        if (len <= 0) continue;
        buf[len] = '\0';

        char *action = NULL, *devname = NULL, *hid_id = NULL;
        for (char *p = buf; p < buf + len; p += strlen(p) + 1) {
            if      (!strncmp(p, "ACTION=", 7)) action = p + 7;
            else if (!strncmp(p, "DEVNAME=", 8)) devname = p + 8;
            else if (!strncmp(p, "HID_ID=", 7))   hid_id = p + 7;
        }

        if (action && devname
            && !strcmp(action, "add")
            && !strncmp(devname, "hidraw", 6)
            && hid_id)
        {
            unsigned int bus, vendor, product;
            if (sscanf(hid_id, "%x:%x:%x", &bus, &vendor, &product) == 3
                && vendor == 0x1050) 
            {
                pid_t pid = fork();
                if (pid == 0) {
                    execl("/usr/local/bin/reprogram_yubi",
                          "reprogram_yubi",
                          devname,
                          (char*)NULL);
                    _exit(1);
                } else if (pid > 0) {
                    waitpid(pid, NULL, 0);
                }
            }
        }
    }

    return 0;
}
