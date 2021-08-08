#include <fcntl.h>
#include <aos/errno.h>
#include <aos/kernel.h>
#include "aos/init.h"
#include "board.h"
#include <k_api.h>
#include "vfs_types.h"
#include "vfs_api.h"

int32_t cloud_fs_register(const char* cloudMonut);
int32_t cloud_fs_unregister(const char *cloudMonut);