#include "sock.h"

#include "csock.h"
#include "log.h"
#include "mem.h"
#include "path.h"

typedef cerr_t (*sock_open_fn)(sock_t *ss, sock_family_t family, sock_type_t type, int protocol, void **sock);
typedef cerr_t (*sock_close_fn)(sock_t *ss, void *sock);

typedef cerr_t (*sock_setopt_fn)(sock_t *ss, void *sock, sock_opt_t opt, void *val, size_t size);

typedef cerr_t (*sock_get_flags_fn)(sock_t *ss, void *sock, int *flags);
typedef cerr_t (*sock_set_flags_fn)(sock_t *ss, void *sock, int flags);

typedef cerr_t (*sock_bind_fn)(sock_t *ss, void *sock, sock_family_t family, strv_t path, size_t len);
typedef cerr_t (*sock_listen_fn)(sock_t *ss, void *sock, int n);
typedef cerr_t (*sock_script_fn)(sock_t *ss, void *sock, const void *data, size_t size);
typedef cerr_t (*sock_connect_fn)(sock_t *ss, void *sock, sock_family_t family, strv_t path, size_t len);
typedef cerr_t (*sock_accept_fn)(sock_t *ss, void *sock, void **fd);

typedef cerr_t (*sock_write_fn)(sock_t *ss, void *sock, const void *data, size_t size, size_t *n);
typedef cerr_t (*sock_read_fn)(sock_t *ss, void *sock, void *data, size_t size, size_t *n);

typedef struct fs_ops_s {
	sock_open_fn open;
	sock_close_fn close;
	sock_setopt_fn setopt;
	sock_get_flags_fn get_flags;
	sock_set_flags_fn set_flags;
	sock_bind_fn bind;
	sock_listen_fn listen;
	sock_script_fn script;
	sock_connect_fn connect;
	sock_accept_fn accept;
	sock_write_fn write;
	sock_read_fn read;
} fs_ops_t;

typedef enum fs_node_flag_e {
	SOCK_NODE_FLAG_OPEN,
	SOCK_NODE_FLAG_BOUND,
	SOCK_NODE_FLAG_LISTEN,
	SOCK_NODE_FLAG_CONNECTED,
	SOCK_NODE_FLAG_ACCEPTED,
	SOCK_NODE_FLAG_NONBLOCK,
} fs_node_flag_t;

typedef struct sock_node_s {
	loc_t path;
	buf_t data;
	buf_t script;
	size_t rcvbuf;
	uint peer;
	uint pending;
	int flags;
} sock_node_t;

static uint sock_id(void *sock)
{
	return (uint)((size_t)sock - 1);
}

static int sock_flag(const sock_node_t *node, fs_node_flag_t flag)
{
	return node->flags & (1 << flag);
}

static void sock_set_flag(sock_node_t *node, fs_node_flag_t flag)
{
	node->flags |= 1 << flag;
}

static void sock_clear_flag(sock_node_t *node, fs_node_flag_t flag)
{
	node->flags &= ~(1 << flag);
}

static sock_node_t *get_open_node(sock_t *ss, void *sock)
{
	sock_node_t *node = arr_get(&ss->nodes, sock_id(sock));
	if (node == NULL || !sock_flag(node, SOCK_NODE_FLAG_OPEN)) {
		return NULL;
	}
	return node;
}

static sock_node_t *find_bound_node(sock_t *ss, strv_t path, uint *id)
{
	uint i = 0;
	sock_node_t *node;
	arr_foreach(&ss->nodes, i, node)
	{
		if (!sock_flag(node, SOCK_NODE_FLAG_OPEN) || !sock_flag(node, SOCK_NODE_FLAG_BOUND)) {
			continue;
		}

		if (strv_eq(buf_get_str(&ss->paths, node->path), path)) {
			if (id) {
				*id = i;
			}
			return node;
		}
	}
	return NULL;
}

static cerr_t osock_open(sock_t *ss, sock_family_t family, sock_type_t type, int protocol, void **sock)
{
	(void)ss;
	return csock_open((csock_family_t)family, (csock_type_t)type, protocol, sock);
}

static cerr_t vsock_open(sock_t *ss, sock_family_t family, sock_type_t type, int protocol, void **sock)
{
	if (sock == NULL) {
		return CERR_VAL;
	}

	switch (family) {
	case SOCK_FAMILY_UNIX: break;
	default: return CERR_VAL;
	}

	switch (type) {
	case SOCK_TYPE_STREAM: break;
	default: return CERR_VAL;
	}

	if (protocol != 0) {
		return CERR_PROTO;
	}

	uint id;
	sock_node_t *node = arr_add(&ss->nodes, &id);
	if (node == NULL) {
		return CERR_MEM;
	}

	*node = (sock_node_t){
		.peer	 = (uint)-1,
		.pending = (uint)-1,
	};
	sock_set_flag(node, SOCK_NODE_FLAG_OPEN);

	*sock = (void *)((size_t)id + 1);
	return CERR_OK;
}

static cerr_t osock_close(sock_t *ss, void *sock)
{
	(void)ss;
	return csock_close(sock);
}

static cerr_t vsock_close(sock_t *ss, void *sock)
{
	if (sock == NULL) {
		return CERR_VAL;
	}

	sock_node_t *node = get_open_node(ss, sock);
	if (node == NULL) {
		return CERR_DESC;
	}

	if (node->data.data) {
		buf_free(&node->data);
	}
	if (node->script.data) {
		buf_free(&node->script);
	}
	node->peer    = (uint)-1;
	node->pending = (uint)-1;
	node->path    = (loc_t){0};
	node->rcvbuf  = 0;
	sock_clear_flag(node, SOCK_NODE_FLAG_OPEN);

	return CERR_OK;
}

static cerr_t osock_setopt(sock_t *ss, void *sock, sock_opt_t opt, void *val, size_t size)
{
	(void)ss;

	if (sock == NULL || val == NULL) {
		return CERR_VAL;
	}

	switch (opt) {
	case SOCK_OPT_SNDBUF: break;
	default: return CERR_VAL;
	}

	return csock_setopt(sock, (csock_opt_t)opt, val, size);
}

static cerr_t vsock_setopt(sock_t *ss, void *sock, sock_opt_t opt, void *val, size_t size)
{
	if (sock == NULL || val == NULL) {
		return CERR_VAL;
	}

	switch (opt) {
	case SOCK_OPT_SNDBUF: break;
	case SOCK_OPT_RCVBUF: {
		if (size != sizeof(size_t)) {
			return CERR_VAL;
		}
		break;
	}
	default: return CERR_VAL;
	}

	sock_node_t *node = get_open_node(ss, sock);
	if (node == NULL) {
		return CERR_DESC;
	}

	if (opt == SOCK_OPT_RCVBUF) {
		node->rcvbuf = *(size_t *)val;
	}

	return CERR_OK;
}

static cerr_t osock_get_flags(sock_t *ss, void *sock, int *flags)
{
	(void)ss;
	return csock_get_flags(sock, flags);
}

static cerr_t vsock_get_flags(sock_t *ss, void *sock, int *flags)
{
	if (sock == NULL || flags == NULL) {
		return CERR_VAL;
	}

	sock_node_t *node = get_open_node(ss, sock);
	if (node == NULL) {
		return CERR_DESC;
	}

	*flags = sock_flag(node, SOCK_NODE_FLAG_NONBLOCK) ? 04000 : 0;
	return CERR_OK;
}

static cerr_t osock_set_flags(sock_t *ss, void *sock, int flags)
{
	(void)ss;
	return csock_set_flags(sock, flags);
}

static cerr_t vsock_set_flags(sock_t *ss, void *sock, int flags)
{
	if (sock == NULL) {
		return CERR_VAL;
	}

	sock_node_t *node = get_open_node(ss, sock);
	if (node == NULL) {
		return CERR_DESC;
	}

	if (flags & 04000) {
		sock_set_flag(node, SOCK_NODE_FLAG_NONBLOCK);
	} else {
		sock_clear_flag(node, SOCK_NODE_FLAG_NONBLOCK);
	}
	return CERR_OK;
}

static cerr_t osock_bind(sock_t *ss, void *sock, sock_family_t family, strv_t path, size_t len)
{
	(void)ss;
	return csock_bind(sock, (csock_family_t)family, path.data, len);
}

static cerr_t vsock_bind(sock_t *ss, void *sock, sock_family_t family, strv_t path, size_t len)
{
	if (sock == NULL || path.data == NULL) {
		return CERR_VAL;
	}

	if (len > 108) {
		return CERR_VAL;
	}

	switch (family) {
	case SOCK_FAMILY_UNIX: break;
	default: return CERR_VAL;
	}

	sock_node_t *node = get_open_node(ss, sock);
	if (node == NULL) {
		return CERR_DESC;
	}

	strv_t bind_path = STRVN(path.data, len > 0 ? len - 1 : path.len);
	if (find_bound_node(ss, bind_path, NULL)) {
		return CERR_EXIST;
	}

	if (buf_add_str(&ss->paths, bind_path, &node->path)) {
		return CERR_MEM;
	}

	sock_set_flag(node, SOCK_NODE_FLAG_BOUND);
	return CERR_OK;
}

static cerr_t osock_listen(sock_t *ss, void *sock, int n)
{
	(void)ss;
	return csock_listen(sock, n);
}

static cerr_t vsock_listen(sock_t *ss, void *sock, int n)
{
	(void)n;

	if (sock == NULL) {
		return CERR_VAL;
	}

	sock_node_t *node = get_open_node(ss, sock);
	if (node == NULL) {
		return CERR_DESC;
	}

	sock_set_flag(node, SOCK_NODE_FLAG_LISTEN);
	return CERR_OK;
}

static cerr_t osock_connect(sock_t *ss, void *sock, sock_family_t family, strv_t path, size_t len)
{
	(void)ss;
	return csock_connect(sock, (csock_family_t)family, path.data, len);
}

static cerr_t osock_script(sock_t *ss, void *sock, const void *data, size_t size)
{
	(void)ss;
	(void)sock;
	(void)data;
	(void)size;
	return CERR_UNSUPPORTED;
}

static cerr_t vsock_script(sock_t *ss, void *sock, const void *data, size_t size)
{
	if (sock == NULL || (data == NULL && size > 0)) {
		return CERR_VAL;
	}

	sock_node_t *node = get_open_node(ss, sock);
	if (node == NULL) {
		return CERR_DESC;
	}
	if (!sock_flag(node, SOCK_NODE_FLAG_LISTEN)) {
		return CERR_STATE;
	}

	if (node->script.data) {
		buf_free(&node->script);
	}
	if (buf_init(&node->script, size > 0 ? size : 1, ss->paths.alloc) == NULL) {
		return CERR_MEM;
	}
	if (size > 0) {
		mem_copy(node->script.data, node->script.size, data, size);
	}
	node->script.used = size;

	return CERR_OK;
}

static cerr_t vsock_connect(sock_t *ss, void *sock, sock_family_t family, strv_t path, size_t len)
{
	if (sock == NULL || path.data == NULL) {
		return CERR_VAL;
	}

	if (len > 108) {
		return CERR_VAL;
	}

	switch (family) {
	case SOCK_FAMILY_UNIX: break;
	default: return CERR_VAL;
	}

	sock_node_t *client = get_open_node(ss, sock);
	if (client == NULL) {
		return CERR_DESC;
	}

	strv_t connect_path = STRVN(path.data, len > 0 ? len - 1 : path.len);
	uint server_id;
	sock_node_t *server = find_bound_node(ss, connect_path, &server_id);
	if (server == NULL) {
		return CERR_NOT_FOUND;
	}
	if (!sock_flag(server, SOCK_NODE_FLAG_LISTEN)) {
		return CERR_CONN;
	}

	uint peer_id;
	sock_node_t *peer = arr_add(&ss->nodes, &peer_id);
	if (peer == NULL) {
		return CERR_MEM;
	}
	*peer = (sock_node_t){
		.peer	 = (uint)-1,
		.pending = (uint)-1,
	};
	server = arr_get(&ss->nodes, server_id);
	client = arr_get(&ss->nodes, sock_id(sock));

	if (server->script.data) {
		if (buf_init(&client->data, server->script.used > 0 ? server->script.used : 1, ss->paths.alloc) == NULL) {
			arr_reset(&ss->nodes, peer_id);
			return CERR_MEM;
		}
		if (server->script.used > 0) {
			mem_copy(client->data.data, client->data.size, server->script.data, server->script.used);
		}
		client->data.used = server->script.used;
	}

	peer->peer = sock_id(sock);
	sock_set_flag(peer, SOCK_NODE_FLAG_OPEN);
	sock_set_flag(peer, SOCK_NODE_FLAG_CONNECTED);

	client->peer = peer_id;
	sock_set_flag(client, SOCK_NODE_FLAG_CONNECTED);

	server->pending = peer_id;

	return CERR_OK;
}

static cerr_t osock_accept(sock_t *ss, void *sock, void **fd)
{
	(void)ss;
	return csock_accept(sock, fd);
}

static cerr_t vsock_accept(sock_t *ss, void *sock, void **fd)
{
	if (sock == NULL || fd == NULL) {
		return CERR_VAL;
	}

	sock_node_t *node = get_open_node(ss, sock);
	if (node == NULL) {
		return CERR_DESC;
	}

	if (!sock_flag(node, SOCK_NODE_FLAG_LISTEN)) {
		return CERR_STATE;
	}

	if (node->pending == (uint)-1) {
		return sock_flag(node, SOCK_NODE_FLAG_NONBLOCK) ? CERR_AGAIN : CERR_STATE;
	}

	uint pending	  = node->pending;
	node->pending	  = (uint)-1;
	sock_node_t *peer = arr_get(&ss->nodes, pending);

	sock_set_flag(peer, SOCK_NODE_FLAG_ACCEPTED);
	*fd = (void *)((size_t)pending + 1);
	return CERR_OK;
}

static cerr_t osock_write(sock_t *ss, void *sock, const void *data, size_t size, size_t *n)
{
	(void)ss;
	return csock_write(sock, data, size, n);
}

static cerr_t vsock_write(sock_t *ss, void *sock, const void *data, size_t size, size_t *n)
{
	if (sock == NULL || data == NULL) {
		return CERR_VAL;
	}

	sock_node_t *node = get_open_node(ss, sock);
	if (node == NULL) {
		return CERR_DESC;
	}
	if (!sock_flag(node, SOCK_NODE_FLAG_CONNECTED)) {
		return CERR_CONN;
	}

	sock_node_t *peer = arr_get(&ss->nodes, node->peer);
	if (peer == NULL || !sock_flag(peer, SOCK_NODE_FLAG_OPEN)) {
		return CERR_CONN;
	}
	if (peer->rcvbuf > 0 && (peer->data.used >= peer->rcvbuf || size > peer->rcvbuf - peer->data.used)) {
		return CERR_MEM;
	}

	if (peer->data.data == NULL && buf_init(&peer->data, size > 0 ? size : 1, ss->paths.alloc) == NULL) {
		return CERR_MEM;
	}

	if (buf_add(&peer->data, size, data, NULL)) {
		return CERR_MEM;
	}

	if (n) {
		*n = size;
	}
	return CERR_OK;
}

static cerr_t osock_read(sock_t *ss, void *sock, void *data, size_t size, size_t *n)
{
	(void)ss;
	return csock_read(sock, data, size, n);
}

static cerr_t vsock_read(sock_t *ss, void *sock, void *data, size_t size, size_t *n)
{
	if (sock == NULL || data == NULL) {
		return CERR_VAL;
	}

	sock_node_t *node = get_open_node(ss, sock);
	if (node == NULL) {
		return CERR_DESC;
	}
	if (!sock_flag(node, SOCK_NODE_FLAG_CONNECTED) ||
	    (sock_flag(node, SOCK_NODE_FLAG_LISTEN) && !sock_flag(node, SOCK_NODE_FLAG_ACCEPTED))) {
		return CERR_STATE;
	}

	sock_node_t *peer = arr_get(&ss->nodes, node->peer);
	if ((peer == NULL || !sock_flag(peer, SOCK_NODE_FLAG_OPEN)) && node->data.used == 0) {
		return CERR_CONN;
	}
	if (node->data.used == 0) {
		return sock_flag(node, SOCK_NODE_FLAG_NONBLOCK) ? CERR_AGAIN : CERR_STATE;
	}

	size_t cnt = size < node->data.used ? size : node->data.used;
	mem_copy(data, size, node->data.data, cnt);
	mem_move(node->data.data, node->data.size, (byte *)node->data.data + cnt, node->data.used - cnt);
	node->data.used -= cnt;
	if (n) {
		*n = cnt;
	}
	return CERR_OK;
}

static const fs_ops_t s_ss_ops[] = {
	[0] =
		{
			.open	   = osock_open,
			.close	   = osock_close,
			.setopt	   = osock_setopt,
			.get_flags = osock_get_flags,
			.set_flags = osock_set_flags,
			.bind	   = osock_bind,
			.listen	   = osock_listen,
			.script	   = osock_script,
			.connect   = osock_connect,
			.accept	   = osock_accept,
			.write	   = osock_write,
			.read	   = osock_read,
		},
	[1] =
		{
			.open	   = vsock_open,
			.close	   = vsock_close,
			.setopt	   = vsock_setopt,
			.get_flags = vsock_get_flags,
			.set_flags = vsock_set_flags,
			.bind	   = vsock_bind,
			.listen	   = vsock_listen,
			.script	   = vsock_script,
			.connect   = vsock_connect,
			.accept	   = vsock_accept,
			.write	   = vsock_write,
			.read	   = vsock_read,
		},
};

sock_t *sock_init(sock_t *ss, uint nodes_cap, int virt, alloc_t alloc)
{
	if (ss == NULL) {
		return NULL;
	}

	if (virt &&
	    (buf_init(&ss->paths, nodes_cap * 16, alloc) == NULL || arr_init(&ss->nodes, nodes_cap, sizeof(sock_node_t), alloc) == NULL)) {
		return NULL;
	}

	ss->virt = virt;

	return ss;
}

void sock_free(sock_t *ss)
{
	if (ss == NULL || !ss->virt) {
		return;
	}

	sock_node_t *node;
	uint i = 0;
	arr_foreach(&ss->nodes, i, node)
	{
		if (node->data.data) {
			buf_free(&node->data);
		}
		if (node->script.data) {
			buf_free(&node->script);
		}
	}

	arr_free(&ss->nodes);
	buf_free(&ss->paths);
}

int sock_open(sock_t *ss, sock_family_t family, sock_type_t type, int protocol, void **sock)
{
	if (ss == NULL) {
		return CERR_VAL;
	}

	cerr_t err = s_ss_ops[ss->virt].open(ss, family, type, protocol, sock);
	if (err) {
		log_error("cutils", "sock", NULL, "failed to open sock: %s", cerr_str(err));
	}

	return err;
}

int sock_close(sock_t *ss, void *sock)
{
	if (ss == NULL) {
		return CERR_VAL;
	}

	cerr_t err = s_ss_ops[ss->virt].close(ss, sock);
	if (err) {
		log_error("cutils", "sock", NULL, "failed to close socket: %s", cerr_str(err));
		return err;
	}

	return CERR_OK;
}

int sock_setopt(sock_t *ss, void *sock, sock_opt_t opt, void *val, size_t size)
{
	if (ss == NULL) {
		return CERR_VAL;
	}

	cerr_t err = s_ss_ops[ss->virt].setopt(ss, sock, opt, val, size);
	if (err) {
		log_error("cutils", "sock", NULL, "failed to setopt: %s", cerr_str(err));
		return err;
	}

	return CERR_OK;
}

int sock_get_flags(sock_t *ss, void *sock, int *flags)
{
	if (ss == NULL) {
		return CERR_VAL;
	}

	cerr_t err = s_ss_ops[ss->virt].get_flags(ss, sock, flags);
	if (err) {
		log_error("cutils", "sock", NULL, "failed to get flags: %s", cerr_str(err));
		return err;
	}

	return CERR_OK;
}

int sock_set_flags(sock_t *ss, void *sock, int flags)
{
	if (ss == NULL) {
		return CERR_VAL;
	}

	cerr_t err = s_ss_ops[ss->virt].set_flags(ss, sock, flags);
	if (err) {
		log_error("cutils", "sock", NULL, "failed to set flags: %s", cerr_str(err));
		return err;
	}

	return CERR_OK;
}

int sock_bind(sock_t *ss, void *sock, sock_family_t family, const char *path, size_t len)
{
	if (ss == NULL) {
		return CERR_VAL;
	}

	cerr_t err = s_ss_ops[ss->virt].bind(ss, sock, family, strv_cstr(path), len);
	if (err) {
		log_error("cutils", "sock", NULL, "failed to bind: %s", cerr_str(err));
		return err;
	}

	return CERR_OK;
}

int sock_listen(sock_t *ss, void *sock, int n)
{
	if (ss == NULL) {
		return CERR_VAL;
	}

	cerr_t err = s_ss_ops[ss->virt].listen(ss, sock, n);
	if (err) {
		log_error("cutils", "sock", NULL, "failed to listen: %s", cerr_str(err));
		return err;
	}

	return CERR_OK;
}

int sock_script(sock_t *ss, void *sock, const void *data, size_t size)
{
	if (ss == NULL) {
		return CERR_VAL;
	}

	cerr_t err = s_ss_ops[ss->virt].script(ss, sock, data, size);
	if (err) {
		log_error("cutils", "sock", NULL, "failed to script: %s", cerr_str(err));
		return err;
	}

	return CERR_OK;
}

int sock_connect(sock_t *ss, void *sock, sock_family_t family, const char *path, size_t len)
{
	if (ss == NULL) {
		return CERR_VAL;
	}

	cerr_t err = s_ss_ops[ss->virt].connect(ss, sock, family, strv_cstr(path), len);
	if (err) {
		log_error("cutils", "sock", NULL, "failed to connect: %s", cerr_str(err));
		return err;
	}

	return CERR_OK;
}

int sock_accept(sock_t *ss, void *sock, void **fd)
{
	if (ss == NULL) {
		return CERR_VAL;
	}

	cerr_t err = s_ss_ops[ss->virt].accept(ss, sock, fd);
	if (err) {
		log_error("cutils", "sock", NULL, "failed to accept: %s", cerr_str(err));
		return err;
	}

	return CERR_OK;
}

int sock_write(sock_t *ss, void *sock, const void *data, size_t size, size_t *n)
{
	if (ss == NULL) {
		return CERR_VAL;
	}

	cerr_t err = s_ss_ops[ss->virt].write(ss, sock, data, size, n);
	if (err) {
		log_error("cutils", "sock", NULL, "failed to write: %s", cerr_str(err));
		return err;
	}

	return CERR_OK;
}

int sock_write_all(sock_t *ss, void *sock, const void *data, size_t size)
{
	const u8 *p = data;

	while (size > 0) {
		size_t n;
		cerr_t err;

		do {
			err = sock_write(ss, sock, p, size, &n);
		} while (err == CERR_INTERRUPT);

		if (err) {
			log_error("cutils", "sock", NULL, "failed to write all: %s", cerr_str(err));
			return err;
		}

		p = &p[n];
		size -= (size_t)n;
	}

	return CERR_OK;
}

int sock_read(sock_t *ss, void *sock, void *data, size_t size, size_t *n)
{
	if (ss == NULL) {
		return CERR_VAL;
	}

	cerr_t err = s_ss_ops[ss->virt].read(ss, sock, data, size, n);
	if (err) {
		if (err != CERR_AGAIN) {
			log_error("cutils", "sock", NULL, "failed to read: %s", cerr_str(err));
		}
		return err;
	}

	return CERR_OK;
}

int sock_read_all(sock_t *ss, void *sock, void *data, size_t size)
{
	u8 *p = data;

	while (size > 0) {
		size_t n;
		cerr_t err;

		do {
			err = sock_read(ss, sock, p, size, &n);
		} while (err == CERR_INTERRUPT);

		if (err) {
			log_error("cutils", "sock", NULL, "failed to read all: %s", cerr_str(err));
			return err;
		}

		p = &p[n];
		size -= (size_t)n;
	}

	return CERR_OK;
}
