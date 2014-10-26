#ifndef _TBUF_H_
#define _TBUF_H_

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>
#include <sys/types.h>

struct tbuf {
    char *data;

    size_t size;
    size_t alloc_size;
};

void tbuf_init(struct tbuf *buf);
void tbuf_reset(struct tbuf *buf);
void tbuf_delete(struct tbuf *buf);
void tbuf_shrink(struct tbuf *buf, size_t size);
void tbuf_ensure(struct tbuf *buf, size_t size);
void tbuf_printf(struct tbuf *buf, const char *format, ...);
void tbuf_append(struct tbuf *buf, const void *data, size_t size);
void tbuf_insert(struct tbuf *buf, const void *data, size_t size);

#define tbuf_printf0(tbuf, fmt, args...) ({ \
    tbuf_printf((tbuf), fmt, ##args);       \
    (tbuf)->size++;                         \
})

#endif
