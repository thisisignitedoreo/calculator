
#include "cril.h"

#include <linux/input.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

cril_id_t cril_init(const char *id_path) {
    int fd = open(id_path, O_RDONLY | O_NONBLOCK);
    if (fd == -1) return (cril_id_t) { .fd = -1, .error = errno };

    return (cril_id_t) { .fd = fd };
}

void cril_deinit(cril_id_t id) {
    if (id.fd != -1) {
        close(id.fd);
    }
}

const char *cril_error(int error) {
    return strerror(error);
}

bool cril_read_input(cril_id_t id, cril_input_t *input) {
    struct input_event event;
    struct input_absinfo absinfo;
    *input = (cril_input_t) {0};

    while (true) {
        if (read(id.fd, &event, sizeof(struct input_event)) != sizeof(struct input_event)) return false;

        if (event.type == EV_KEY) {
            input->type = event.value;
            input->key = event.code;
            break;
        }
        if (event.type == EV_ABS) {
            ioctl(id.fd, EVIOCGABS(event.code), &absinfo);
            input->type = CRIL_ANALOG;
            input->axis = event.code;
            input->min = absinfo.minimum;
            input->max = absinfo.maximum;
            input->value = event.value;
            break;
        }
    }
    return true;
}
