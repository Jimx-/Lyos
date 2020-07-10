#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <lyos/const.h>
#include <lyos/types.h>
#include <lyos/service.h>

#include "cJSON.h"

#include "parse.h"

static void parse_pci_class(cJSON* root, struct service_up_req* up_req)
{
    up_req->nr_pci_class = 0;

    cJSON* classes = cJSON_GetObjectItem(root, PCI_CLASS);
    if (!classes) return;

    int i;
    cJSON* _class;
    for (i = 0; i < cJSON_GetArraySize(classes); i++) {
        _class = cJSON_GetArrayItem(classes, i);

        if (_class->type != cJSON_String) continue;

        char* str = _class->valuestring;
        char* check;

        unsigned char base_class = strtoul(str, &check, 0x10);
        unsigned char sub_class = 0, interface = 0;
        unsigned int mask = 0xff0000;

        if (*check == '/') {
            sub_class = strtoul(check + 1, &check, 0x10);
            mask = 0xffff00;
            if (*check == '/') {
                interface = strtoul(check + 1, &check, 0x10);
                mask = 0xffffff;
            }
        }

        if (*check != '\0') continue;

        up_req->pci_class[up_req->nr_pci_class].classid =
            (base_class << 16) | (sub_class << 8) | interface;
        up_req->pci_class[up_req->nr_pci_class].mask = mask;
        up_req->nr_pci_class++;
    }
}

static void parse_pci_device(cJSON* root, struct service_up_req* up_req)
{
    up_req->nr_pci_id = 0;

    cJSON* devices = cJSON_GetObjectItem(root, PCI_DEVICE);
    if (!devices) return;

    int i;
    cJSON* device;
    for (i = 0; i < cJSON_GetArraySize(devices); i++) {
        device = cJSON_GetArrayItem(devices, i);

        if (device->type != cJSON_String) continue;

        char* str = device->valuestring;
        char* check;

        u16 vid = strtoul(str, &check, 0x10);
        u16 did = 0;

        if (*check == ':') {
            did = strtoul(check + 1, &check, 0x10);
        }

        if (*check != '\0') continue;

        up_req->pci_id[up_req->nr_pci_id].vid = vid;
        up_req->pci_id[up_req->nr_pci_id].did = did;
        up_req->nr_pci_id++;
    }
}

int parse_config(char* progname, char* path, struct service_up_req* up_req)
{
    struct stat stat_buf;
    int r;
    if ((r = stat(path, &stat_buf)) != 0) return r;

    char* buf = (char*)malloc(stat_buf.st_size);
    if (!buf) return ENOMEM;

    int fd = open(path, O_RDONLY);
    read(fd, buf, stat_buf.st_size);
    close(fd);

    cJSON* root = cJSON_Parse(buf);
    if (!root) {
        free(buf);
        return EINVAL;
    }

    parse_pci_class(root, up_req);
    parse_pci_device(root, up_req);

    cJSON_Delete(root);
    free(buf);

    return 0;
}
