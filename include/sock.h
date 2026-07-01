#ifndef SOCK_H
#define SOCK_H

#include "arr.h"
#include "buf.h"

typedef enum sock_family_e {
	SOCK_FAMILY_UNKNOWN,
	SOCK_FAMILY_UNIX,
} sock_family_t;

typedef enum sock_type_e {
	SOCK_TYPE_UNKNOWN,
	SOCK_TYPE_STREAM,
} sock_type_t;

typedef enum sock_opt_e {
	SOCK_OPT_UNKNOWN,
	SOCK_OPT_SNDBUF,
} sock_opt_t;

typedef struct sock_s {
	buf_t paths;
	arr_t nodes;
	int virt;
} sock_t;

sock_t *sock_init(sock_t *ss, uint nodes_cap, int virt, alloc_t alloc);
void sock_free(sock_t *ss);

int sock_open(sock_t *ss, sock_family_t family, sock_type_t type, int protocol, void **sock);
int sock_close(sock_t *ss, void *sock);

int sock_setopt(sock_t *ss, void *sock, sock_opt_t opt, void *val, size_t size);
int sock_get_flags(sock_t *ss, void *sock, int *flags);
int sock_set_flags(sock_t *ss, void *sock, int flags);
int sock_bind(sock_t *ss, void *sock, sock_family_t family, const char *path, size_t len);
int sock_listen(sock_t *ss, void *sock, int n);
int sock_connect(sock_t *ss, void *sock, sock_family_t family, const char *path, size_t len);
int sock_accept(sock_t *ss, void *sock, void **fd);
int sock_write(sock_t *ss, void *sock, const void *data, size_t size, size_t *n);
int sock_read(sock_t *ss, void *sock, void *data, size_t size, size_t *n);

#endif
