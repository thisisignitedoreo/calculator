
#ifndef CRIL_H_
#define CRIL_H_

#include <stdbool.h>
#include <linux/input-event-codes.h>

typedef struct cril_id {
    int fd, error;
} cril_id_t;

cril_id_t cril_init(const char *id_path);
void cril_deinit(cril_id_t id);

const char *cril_error(int error);

typedef struct input {
    enum {
        CRIL_RELEASE = 0,
        CRIL_HOLD = 1,
        CRIL_AUTOREPEAT = 2,
        CRIL_ANALOG = 3,
    } type;
    int key; // type 0..3
    int axis, min, max, value; // type 4
} cril_input_t;

bool cril_read_input(cril_id_t id, cril_input_t *input);

#endif /* CRIL_H_ */
