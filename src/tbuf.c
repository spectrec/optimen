#include <stdio.h>

#include "tbuf.h"

void tbuf_init(struct tbuf *buf)
{
	assert(buf != NULL);

	buf->data = NULL;
	buf->size = 0;
	buf->alloc_size = 0;
}

void tbuf_delete(struct tbuf *buf)
{
	assert(buf != NULL);

	if (buf->data == NULL)
		return;

	free(buf->data);
	tbuf_init(buf);
}

void tbuf_reset(struct tbuf *buf)
{
	assert(buf != NULL);
	buf->size = 0;
}

void tbuf_ensure(struct tbuf *buf, size_t size)
{
	if (buf->size + size >= buf->alloc_size) {
		size_t new_size = buf->alloc_size * 2;
		if (new_size == 0)
			new_size = size + 1;

		while (buf->size + size >= new_size)
			new_size *= 2;

		char *new_buf = (char *)realloc(buf->data, new_size);
		assert(new_buf != NULL);

		buf->data = new_buf;
		buf->alloc_size = new_size;
	}
}

void tbuf_append(struct tbuf *buf, const void *data, size_t size)
{
	tbuf_ensure(buf, size);
	memcpy(buf->data + buf->size, data, size);
	buf->size += size;
}

void tbuf_shrink(struct tbuf *buf, size_t size)
{
	if (size > buf->size) {
		buf->size = 0;
		return;
	}

	memmove(buf->data, buf->data + size, buf->size - size);
	buf->size -= size;
}

void tbuf_insert(struct tbuf *buf, const void *data, size_t size)
{
	tbuf_reset(buf);
	tbuf_append(buf, data, size);
}

void tbuf_printf(struct tbuf *buf, const char *format, ...)
{
	va_list ap, ap_cpy;
	va_start(ap, format);
	va_copy(ap_cpy, ap);

	int len = vsnprintf(NULL, 0, format, ap_cpy);
	va_end(ap_cpy);

	tbuf_ensure(buf, len);
	vsprintf(buf->data + buf->size, format, ap);
	buf->size += len;
	va_end(ap);
}
