#if AOS_COMP_CLI
#include "aos/cli.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "cloud_fs.h"

static void cloud_read(int argc, char **argv)
{
    if (argc < 2) {
        printf("Command has a problem!\r\n");
        return;
    }
    std::string filepath = argv[1];
    printf("filepath:%s\r\n", argv[1]);
    if (filepath.length() >= 1024) {
        printf("filepath is too long!\r\n");
    }
    else if (filepath.find('/') != std::string::npos) {
        printf("Sorry, wo don't support directories now!\r\n");
    }
    else {
        cloud_fs_read(argv[1]);
    }
}

#if AOS_COMP_CLI
/* reg args: fun, cmd, description*/
ALIOS_CLI_CMD_REGISTER(cloud_read, cloud_read, cloud_read_example)
#endif