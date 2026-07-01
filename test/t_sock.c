#include "sock.h"

#include "cerr.h"
#include "cfs.h"
#include "platform.h"
#include "log.h"
#include "mem.h"
#include "test.h"

#define TEST_SOCK "/tmp/cutils_t_sock.sock"
#define TEST_SOCK_WRITE "/tmp/cutils_t_sock_write.sock"

#if defined(C_LINUX)
	#define T_OSOCK_EXPECT(err) (err)
#else
	#define T_OSOCK_EXPECT(err) CERR_UNSUPPORTED
#endif

typedef struct t_sock_pair_s {
	sock_t ss;
	sock_t vss;
	void *s;
	void *vs;
} t_sock_pair_t;

typedef struct t_sock_conn_s {
	sock_t ss;
	sock_t vss;
	void *s;
	void *vs;
	void *c;
	void *vc;
	void *p;
	void *vp;
} t_sock_conn_t;

static void t_sock_cleanup()
{
#if defined(C_LINUX)
	cfs_unlink(TEST_SOCK);
	cfs_unlink(TEST_SOCK_WRITE);
#endif
}

static void t_sock_init(sock_t *ss, sock_t *vss)
{
	sock_init(ss, 0, 0, ALLOC_STD);
	sock_init(vss, 8, 1, ALLOC_STD);
}

static void t_sock_free(sock_t *ss, sock_t *vss)
{
	sock_free(ss);
	sock_free(vss);
}

static void t_sock_pair_open(t_sock_pair_t *p)
{
	t_sock_init(&p->ss, &p->vss);
#if defined(C_LINUX)
	sock_open(&p->ss, SOCK_FAMILY_UNIX, SOCK_TYPE_STREAM, 0, &p->s);
#else
	p->s = (void *)1;
#endif
	sock_open(&p->vss, SOCK_FAMILY_UNIX, SOCK_TYPE_STREAM, 0, &p->vs);
}

static void t_sock_pair_close(t_sock_pair_t *p)
{
#if defined(C_LINUX)
	sock_close(&p->ss, p->s);
#endif
	sock_close(&p->vss, p->vs);
	t_sock_free(&p->ss, &p->vss);
}

static void t_sock_conn_open(t_sock_conn_t *c)
{
	t_sock_cleanup();
	t_sock_init(&c->ss, &c->vss);
#if defined(C_LINUX)
	sock_open(&c->ss, SOCK_FAMILY_UNIX, SOCK_TYPE_STREAM, 0, &c->s);
	sock_open(&c->ss, SOCK_FAMILY_UNIX, SOCK_TYPE_STREAM, 0, &c->c);
#else
	c->s = (void *)1;
	c->c = (void *)2;
	c->p = (void *)3;
#endif
	sock_open(&c->vss, SOCK_FAMILY_UNIX, SOCK_TYPE_STREAM, 0, &c->vs);
	sock_open(&c->vss, SOCK_FAMILY_UNIX, SOCK_TYPE_STREAM, 0, &c->vc);
}

static void t_sock_conn_listen(t_sock_conn_t *c)
{
#if defined(C_LINUX)
	sock_bind(&c->ss, c->s, SOCK_FAMILY_UNIX, TEST_SOCK, sizeof(TEST_SOCK));
	sock_listen(&c->ss, c->s, 1);
#endif
	sock_bind(&c->vss, c->vs, SOCK_FAMILY_UNIX, TEST_SOCK, sizeof(TEST_SOCK));
	sock_listen(&c->vss, c->vs, 1);
}

static void t_sock_conn_connect(t_sock_conn_t *c)
{
	t_sock_conn_listen(c);
#if defined(C_LINUX)
	sock_connect(&c->ss, c->c, SOCK_FAMILY_UNIX, TEST_SOCK, sizeof(TEST_SOCK));
	sock_accept(&c->ss, c->s, &c->p);
#endif
	sock_connect(&c->vss, c->vc, SOCK_FAMILY_UNIX, TEST_SOCK, sizeof(TEST_SOCK));
	sock_accept(&c->vss, c->vs, &c->vp);
}

static void t_sock_conn_close(t_sock_conn_t *c)
{
#if defined(C_LINUX)
	if (c->p) {
		sock_close(&c->ss, c->p);
	}
	sock_close(&c->ss, c->c);
	sock_close(&c->ss, c->s);
#endif
	if (c->vp) {
		sock_close(&c->vss, c->vp);
	}
	sock_close(&c->vss, c->vc);
	sock_close(&c->vss, c->vs);
	t_sock_free(&c->ss, &c->vss);
	t_sock_cleanup();
}

TEST(sock_init_free)
{
	START;

	sock_t ss = {0};

	EXPECT_EQ(sock_init(NULL, 0, 0, ALLOC_STD), NULL);
	mem_oom(1);
	EXPECT_EQ(sock_init(&ss, 1, 1, ALLOC_STD), NULL);
	mem_oom(0);
	EXPECT_EQ(sock_init(&ss, 1, 1, ALLOC_STD), &ss);

	sock_free(NULL);
	sock_free(&ss);

	END;
}

TEST(sock_open_invalid)
{
	START;

	sock_t ss  = {0};
	sock_t vss = {0};
	void *sock;

	sock_init(&ss, 0, 0, ALLOC_STD);
	sock_init(&vss, 1, 1, ALLOC_STD);

	EXPECT_EQ(sock_open(NULL, -1, -1, -1, NULL), CERR_VAL);
	log_set_quiet(0, 1);
	EXPECT_EQ(sock_open(&ss, -1, -1, -1, &sock), T_OSOCK_EXPECT(CERR_VAL));
	EXPECT_EQ(sock_open(&vss, -1, -1, -1, &sock), CERR_VAL);
	EXPECT_EQ(sock_open(&ss, SOCK_FAMILY_UNIX, SOCK_TYPE_STREAM, -1, &sock), T_OSOCK_EXPECT(CERR_PROTO));
	EXPECT_EQ(sock_open(&vss, SOCK_FAMILY_UNIX, SOCK_TYPE_STREAM, -1, &sock), CERR_PROTO);
	EXPECT_EQ(sock_open(&ss, SOCK_FAMILY_UNIX, -1, 0, &sock), T_OSOCK_EXPECT(CERR_VAL));
	EXPECT_EQ(sock_open(&vss, SOCK_FAMILY_UNIX, -1, 0, &sock), CERR_VAL);
	EXPECT_EQ(sock_open(&ss, SOCK_FAMILY_UNIX, SOCK_TYPE_STREAM, 0, NULL), CERR_VAL);
	EXPECT_EQ(sock_open(&vss, SOCK_FAMILY_UNIX, SOCK_TYPE_STREAM, 0, NULL), CERR_VAL);
	log_set_quiet(0, 0);

	sock_free(&ss);
	sock_free(&vss);

	END;
}

TEST(sock_open_oom)
{
	START;

	sock_t vss = {0};
	void *sock;

	log_set_quiet(0, 1);
	sock_init(&vss, 0, 1, ALLOC_STD);
	log_set_quiet(0, 0);

	mem_oom(1);
	EXPECT_EQ(sock_open(&vss, SOCK_FAMILY_UNIX, SOCK_TYPE_STREAM, 0, &sock), CERR_MEM);
	mem_oom(0);

	sock_free(&vss);

	END;
}

TEST(sock_close_invalid)
{
	START;

	t_sock_pair_t p = {0};
	t_sock_pair_open(&p);

	EXPECT_EQ(sock_close(NULL, NULL), CERR_VAL);
	log_set_quiet(0, 1);
	EXPECT_EQ(sock_close(&p.ss, NULL), CERR_VAL);
	EXPECT_EQ(sock_close(&p.vss, NULL), CERR_VAL);
	EXPECT_EQ(sock_close(&p.ss, (void *)-1), T_OSOCK_EXPECT(CERR_DESC));
	EXPECT_EQ(sock_close(&p.vss, (void *)-1), CERR_DESC);
	log_set_quiet(0, 0);

	t_sock_pair_close(&p);

	END;
}

TEST(sock_close_valid)
{
	START;

	t_sock_pair_t p = {0};
	t_sock_pair_open(&p);

	EXPECT_EQ(sock_close(&p.ss, p.s), T_OSOCK_EXPECT(CERR_OK));
	EXPECT_EQ(sock_close(&p.vss, p.vs), CERR_OK);
	t_sock_free(&p.ss, &p.vss);

	END;
}

TEST(sock_setopt_invalid)
{
	START;

	t_sock_pair_t p = {0};
	int size	= 4096;
	t_sock_pair_open(&p);

	EXPECT_EQ(sock_setopt(NULL, NULL, -1, NULL, 0), CERR_VAL);
	log_set_quiet(0, 1);
	EXPECT_EQ(sock_setopt(&p.ss, NULL, SOCK_OPT_SNDBUF, &size, sizeof(size)), CERR_VAL);
	EXPECT_EQ(sock_setopt(&p.vss, NULL, SOCK_OPT_SNDBUF, &size, sizeof(size)), CERR_VAL);
	EXPECT_EQ(sock_setopt(&p.ss, p.s, SOCK_OPT_SNDBUF, NULL, sizeof(size)), CERR_VAL);
	EXPECT_EQ(sock_setopt(&p.vss, p.vs, SOCK_OPT_SNDBUF, NULL, sizeof(size)), CERR_VAL);
	EXPECT_EQ(sock_setopt(&p.ss, p.s, -1, &size, sizeof(size)), T_OSOCK_EXPECT(CERR_VAL));
	EXPECT_EQ(sock_setopt(&p.vss, p.vs, -1, &size, sizeof(size)), CERR_VAL);
	EXPECT_EQ(sock_setopt(&p.ss, (void *)-1, SOCK_OPT_SNDBUF, &size, sizeof(size)), T_OSOCK_EXPECT(CERR_DESC));
	EXPECT_EQ(sock_setopt(&p.vss, (void *)-1, SOCK_OPT_SNDBUF, &size, sizeof(size)), CERR_DESC);
	log_set_quiet(0, 0);

	t_sock_pair_close(&p);

	END;
}

TEST(sock_setopt_valid)
{
	START;

	t_sock_pair_t p = {0};
	int size	= 4096;
	t_sock_pair_open(&p);

	EXPECT_EQ(sock_setopt(&p.ss, p.s, SOCK_OPT_SNDBUF, &size, sizeof(size)), T_OSOCK_EXPECT(CERR_OK));
	EXPECT_EQ(sock_setopt(&p.vss, p.vs, SOCK_OPT_SNDBUF, &size, sizeof(size)), CERR_OK);

	t_sock_pair_close(&p);

	END;
}

TEST(sock_get_flags_invalid)
{
	START;

	t_sock_pair_t p = {0};
	int flags;
	t_sock_pair_open(&p);

	EXPECT_EQ(sock_get_flags(NULL, NULL, NULL), CERR_VAL);
	log_set_quiet(0, 1);
	EXPECT_EQ(sock_get_flags(&p.ss, NULL, &flags), CERR_VAL);
	EXPECT_EQ(sock_get_flags(&p.vss, NULL, &flags), CERR_VAL);
	EXPECT_EQ(sock_get_flags(&p.ss, p.s, NULL), CERR_VAL);
	EXPECT_EQ(sock_get_flags(&p.vss, p.vs, NULL), CERR_VAL);
	EXPECT_EQ(sock_get_flags(&p.ss, (void *)-1, &flags), T_OSOCK_EXPECT(CERR_DESC));
	EXPECT_EQ(sock_get_flags(&p.vss, (void *)-1, &flags), CERR_DESC);
	log_set_quiet(0, 0);

	t_sock_pair_close(&p);

	END;
}

TEST(sock_get_flags_valid)
{
	START;

	t_sock_pair_t p = {0};
	int flags, vflags;
	t_sock_pair_open(&p);

	EXPECT_EQ(sock_get_flags(&p.ss, p.s, &flags), T_OSOCK_EXPECT(CERR_OK));
	EXPECT_EQ(sock_get_flags(&p.vss, p.vs, &vflags), CERR_OK);

	t_sock_pair_close(&p);

	END;
}

TEST(sock_set_flags_invalid)
{
	START;

	t_sock_pair_t p = {0};
	t_sock_pair_open(&p);

	EXPECT_EQ(sock_set_flags(NULL, NULL, 0), CERR_VAL);
	log_set_quiet(0, 1);
	EXPECT_EQ(sock_set_flags(&p.ss, NULL, 0), CERR_VAL);
	EXPECT_EQ(sock_set_flags(&p.vss, NULL, 0), CERR_VAL);
	EXPECT_EQ(sock_set_flags(&p.ss, (void *)-1, 0), T_OSOCK_EXPECT(CERR_DESC));
	EXPECT_EQ(sock_set_flags(&p.vss, (void *)-1, 0), CERR_DESC);
	log_set_quiet(0, 0);

	t_sock_pair_close(&p);

	END;
}

TEST(sock_set_flags_nonblock)
{
	START;

	t_sock_pair_t p = {0};
	int flags = 0, vflags;
	t_sock_pair_open(&p);

	EXPECT_EQ(sock_get_flags(&p.ss, p.s, &flags), T_OSOCK_EXPECT(CERR_OK));
	EXPECT_EQ(sock_get_flags(&p.vss, p.vs, &vflags), CERR_OK);
	EXPECT_EQ(sock_set_flags(&p.ss, p.s, flags | 04000), T_OSOCK_EXPECT(CERR_OK));
	EXPECT_EQ(sock_set_flags(&p.vss, p.vs, vflags | 04000), CERR_OK);
	EXPECT_EQ(sock_get_flags(&p.ss, p.s, &flags), T_OSOCK_EXPECT(CERR_OK));
	EXPECT_EQ(sock_get_flags(&p.vss, p.vs, &vflags), CERR_OK);
#if defined(C_LINUX)
	EXPECT_EQ(flags & 04000, 04000);
#endif
	EXPECT_EQ(vflags & 04000, 04000);
	EXPECT_EQ(sock_set_flags(&p.ss, p.s, 0), T_OSOCK_EXPECT(CERR_OK));
	EXPECT_EQ(sock_set_flags(&p.vss, p.vs, 0), CERR_OK);
	EXPECT_EQ(sock_get_flags(&p.ss, p.s, &flags), T_OSOCK_EXPECT(CERR_OK));
	EXPECT_EQ(sock_get_flags(&p.vss, p.vs, &vflags), CERR_OK);
#if defined(C_LINUX)
	EXPECT_EQ(flags & 04000, 0);
#endif
	EXPECT_EQ(vflags & 04000, 0);

	t_sock_pair_close(&p);

	END;
}

TEST(sock_bind_invalid)
{
	START;

	t_sock_pair_t p = {0};
	char path[109] = {0};
	t_sock_pair_open(&p);

	log_set_quiet(0, 1);
	EXPECT_EQ(sock_bind(NULL, NULL, SOCK_FAMILY_UNIX, TEST_SOCK, sizeof(TEST_SOCK)), CERR_VAL);
	EXPECT_EQ(sock_bind(&p.ss, NULL, SOCK_FAMILY_UNIX, TEST_SOCK, sizeof(TEST_SOCK)), CERR_VAL);
	EXPECT_EQ(sock_bind(&p.vss, NULL, SOCK_FAMILY_UNIX, TEST_SOCK, sizeof(TEST_SOCK)), CERR_VAL);
	EXPECT_EQ(sock_bind(&p.ss, p.s, -1, NULL, 0), CERR_VAL);
	EXPECT_EQ(sock_bind(&p.vss, p.vs, -1, NULL, 0), CERR_VAL);
	EXPECT_EQ(sock_bind(&p.ss, (void *)-1, SOCK_FAMILY_UNIX, TEST_SOCK, sizeof(TEST_SOCK)), T_OSOCK_EXPECT(CERR_DESC));
	EXPECT_EQ(sock_bind(&p.vss, (void *)-1, SOCK_FAMILY_UNIX, TEST_SOCK, sizeof(TEST_SOCK)), CERR_DESC);
	EXPECT_EQ(sock_bind(&p.ss, p.s, SOCK_FAMILY_UNIX, path, sizeof(path)), T_OSOCK_EXPECT(CERR_VAL));
	EXPECT_EQ(sock_bind(&p.vss, p.vs, SOCK_FAMILY_UNIX, path, sizeof(path)), CERR_VAL);
	EXPECT_EQ(sock_bind(&p.ss, p.s, -1, TEST_SOCK, sizeof(TEST_SOCK)), T_OSOCK_EXPECT(CERR_VAL));
	EXPECT_EQ(sock_bind(&p.vss, p.vs, -1, TEST_SOCK, sizeof(TEST_SOCK)), CERR_VAL);
	log_set_quiet(0, 0);

	t_sock_pair_close(&p);

	END;
}

TEST(sock_bind_valid)
{
	START;

	t_sock_pair_t p = {0};
	t_sock_cleanup();
	t_sock_pair_open(&p);

	EXPECT_EQ(sock_bind(&p.ss, p.s, SOCK_FAMILY_UNIX, TEST_SOCK, sizeof(TEST_SOCK)), T_OSOCK_EXPECT(CERR_OK));
	EXPECT_EQ(sock_bind(&p.vss, p.vs, SOCK_FAMILY_UNIX, TEST_SOCK, sizeof(TEST_SOCK)), CERR_OK);

	t_sock_pair_close(&p);
	t_sock_cleanup();

	END;
}

TEST(sock_bind_duplicate)
{
	START;

	t_sock_conn_t c = {0};
	void *other, *vother;
	t_sock_conn_open(&c);

#if defined(C_LINUX)
	sock_bind(&c.ss, c.s, SOCK_FAMILY_UNIX, TEST_SOCK, sizeof(TEST_SOCK));
	sock_open(&c.ss, SOCK_FAMILY_UNIX, SOCK_TYPE_STREAM, 0, &other);
#else
	other = (void *)4;
#endif
	sock_bind(&c.vss, c.vs, SOCK_FAMILY_UNIX, TEST_SOCK, sizeof(TEST_SOCK));
	sock_open(&c.vss, SOCK_FAMILY_UNIX, SOCK_TYPE_STREAM, 0, &vother);

	log_set_quiet(0, 1);
	EXPECT_EQ(sock_bind(&c.ss, other, SOCK_FAMILY_UNIX, TEST_SOCK, sizeof(TEST_SOCK)), T_OSOCK_EXPECT(CERR_EXIST));
	EXPECT_EQ(sock_bind(&c.vss, vother, SOCK_FAMILY_UNIX, TEST_SOCK, sizeof(TEST_SOCK)), CERR_EXIST);
	log_set_quiet(0, 0);

#if defined(C_LINUX)
	sock_close(&c.ss, other);
#endif
	sock_close(&c.vss, vother);
	t_sock_conn_close(&c);

	END;
}

TEST(sock_bind_oom)
{
	START;

	sock_t vss = {0};
	void *sock;

	sock_init(&vss, 1, 1, ALLOC_STD);
	sock_open(&vss, SOCK_FAMILY_UNIX, SOCK_TYPE_STREAM, 0, &sock);
	mem_oom(1);
	log_set_quiet(0, 1);
	EXPECT_EQ(sock_bind(&vss, sock, SOCK_FAMILY_UNIX, TEST_SOCK, sizeof(TEST_SOCK)), CERR_MEM);
	log_set_quiet(0, 0);
	mem_oom(0);

	sock_close(&vss, sock);
	sock_free(&vss);

	END;
}

TEST(sock_listen_invalid)
{
	START;

	t_sock_pair_t p = {0};
	t_sock_pair_open(&p);

	EXPECT_EQ(sock_listen(NULL, NULL, 0), CERR_VAL);
	log_set_quiet(0, 1);
	EXPECT_EQ(sock_listen(&p.ss, NULL, 1), CERR_VAL);
	EXPECT_EQ(sock_listen(&p.vss, NULL, 1), CERR_VAL);
	EXPECT_EQ(sock_listen(&p.ss, (void *)-1, 1), T_OSOCK_EXPECT(CERR_DESC));
	EXPECT_EQ(sock_listen(&p.vss, (void *)-1, 1), CERR_DESC);
	log_set_quiet(0, 0);

	t_sock_pair_close(&p);

	END;
}

TEST(sock_listen_valid)
{
	START;

	t_sock_pair_t p = {0};
	t_sock_cleanup();
	t_sock_pair_open(&p);
#if defined(C_LINUX)
	sock_bind(&p.ss, p.s, SOCK_FAMILY_UNIX, TEST_SOCK, sizeof(TEST_SOCK));
#endif
	sock_bind(&p.vss, p.vs, SOCK_FAMILY_UNIX, TEST_SOCK, sizeof(TEST_SOCK));

	EXPECT_EQ(sock_listen(&p.ss, p.s, 1), T_OSOCK_EXPECT(CERR_OK));
	EXPECT_EQ(sock_listen(&p.vss, p.vs, 1), CERR_OK);

	t_sock_pair_close(&p);
	t_sock_cleanup();

	END;
}

TEST(sock_connect_invalid)
{
	START;

	t_sock_conn_t c = {0};
	char path[109] = {0};
	t_sock_conn_open(&c);

	log_set_quiet(0, 1);
	EXPECT_EQ(sock_connect(NULL, NULL, SOCK_FAMILY_UNIX, TEST_SOCK, sizeof(TEST_SOCK)), CERR_VAL);
	EXPECT_EQ(sock_connect(&c.ss, NULL, SOCK_FAMILY_UNIX, TEST_SOCK, sizeof(TEST_SOCK)), CERR_VAL);
	EXPECT_EQ(sock_connect(&c.vss, NULL, SOCK_FAMILY_UNIX, TEST_SOCK, sizeof(TEST_SOCK)), CERR_VAL);
	EXPECT_EQ(sock_connect(&c.ss, c.c, -1, NULL, 0), CERR_VAL);
	EXPECT_EQ(sock_connect(&c.vss, c.vc, -1, NULL, 0), CERR_VAL);
	EXPECT_EQ(sock_connect(&c.ss, (void *)-1, SOCK_FAMILY_UNIX, TEST_SOCK, sizeof(TEST_SOCK)), T_OSOCK_EXPECT(CERR_DESC));
	EXPECT_EQ(sock_connect(&c.vss, (void *)-1, SOCK_FAMILY_UNIX, TEST_SOCK, sizeof(TEST_SOCK)), CERR_DESC);
	EXPECT_EQ(sock_connect(&c.ss, c.c, SOCK_FAMILY_UNIX, path, sizeof(path)), T_OSOCK_EXPECT(CERR_VAL));
	EXPECT_EQ(sock_connect(&c.vss, c.vc, SOCK_FAMILY_UNIX, path, sizeof(path)), CERR_VAL);
	EXPECT_EQ(sock_connect(&c.ss, c.c, -1, TEST_SOCK, sizeof(TEST_SOCK)), T_OSOCK_EXPECT(CERR_VAL));
	EXPECT_EQ(sock_connect(&c.vss, c.vc, -1, TEST_SOCK, sizeof(TEST_SOCK)), CERR_VAL);
	log_set_quiet(0, 0);

	t_sock_conn_close(&c);

	END;
}

TEST(sock_connect_not_found)
{
	START;

	t_sock_conn_t c = {0};
	t_sock_conn_open(&c);

	log_set_quiet(0, 1);
	EXPECT_EQ(sock_connect(&c.ss, c.c, SOCK_FAMILY_UNIX, "/tmp/cutils_not_found.sock", sizeof("/tmp/cutils_not_found.sock")), T_OSOCK_EXPECT(CERR_NOT_FOUND));
	EXPECT_EQ(sock_connect(&c.vss, c.vc, SOCK_FAMILY_UNIX, "/tmp/cutils_not_found.sock", sizeof("/tmp/cutils_not_found.sock")), CERR_NOT_FOUND);
	log_set_quiet(0, 0);

	t_sock_conn_close(&c);

	END;
}

TEST(sock_connect_not_listening)
{
	START;

	t_sock_conn_t c = {0};
	t_sock_conn_open(&c);
#if defined(C_LINUX)
	sock_bind(&c.ss, c.s, SOCK_FAMILY_UNIX, TEST_SOCK, sizeof(TEST_SOCK));
#endif
	sock_bind(&c.vss, c.vs, SOCK_FAMILY_UNIX, TEST_SOCK, sizeof(TEST_SOCK));

	log_set_quiet(0, 1);
	EXPECT_EQ(sock_connect(&c.ss, c.c, SOCK_FAMILY_UNIX, TEST_SOCK, sizeof(TEST_SOCK)), T_OSOCK_EXPECT(CERR_CONN));
	EXPECT_EQ(sock_connect(&c.vss, c.vc, SOCK_FAMILY_UNIX, TEST_SOCK, sizeof(TEST_SOCK)), CERR_CONN);
	log_set_quiet(0, 0);

	t_sock_conn_close(&c);

	END;
}

TEST(sock_connect_oom)
{
	START;

	sock_t vss = {0};
	void *s, *c;

	sock_init(&vss, 2, 1, ALLOC_STD);
	sock_open(&vss, SOCK_FAMILY_UNIX, SOCK_TYPE_STREAM, 0, &s);
	sock_open(&vss, SOCK_FAMILY_UNIX, SOCK_TYPE_STREAM, 0, &c);
	sock_bind(&vss, s, SOCK_FAMILY_UNIX, TEST_SOCK, sizeof(TEST_SOCK));
	sock_listen(&vss, s, 1);

	mem_oom(1);
	log_set_quiet(0, 1);
	EXPECT_EQ(sock_connect(&vss, c, SOCK_FAMILY_UNIX, TEST_SOCK, sizeof(TEST_SOCK)), CERR_MEM);
	log_set_quiet(0, 0);
	mem_oom(0);

	sock_close(&vss, c);
	sock_close(&vss, s);
	sock_free(&vss);

	END;
}

TEST(sock_connect_valid)
{
	START;

	t_sock_conn_t c = {0};
	t_sock_conn_open(&c);
	t_sock_conn_listen(&c);

	EXPECT_EQ(sock_connect(&c.ss, c.c, SOCK_FAMILY_UNIX, TEST_SOCK, sizeof(TEST_SOCK)), T_OSOCK_EXPECT(CERR_OK));
	EXPECT_EQ(sock_connect(&c.vss, c.vc, SOCK_FAMILY_UNIX, TEST_SOCK, sizeof(TEST_SOCK)), CERR_OK);

	t_sock_conn_close(&c);

	END;
}

TEST(sock_accept_invalid)
{
	START;

	t_sock_conn_t c = {0};
	t_sock_conn_open(&c);
	t_sock_conn_listen(&c);

	EXPECT_EQ(sock_accept(NULL, NULL, NULL), CERR_VAL);
	log_set_quiet(0, 1);
	EXPECT_EQ(sock_accept(&c.ss, NULL, &c.p), CERR_VAL);
	EXPECT_EQ(sock_accept(&c.vss, NULL, &c.vp), CERR_VAL);
	EXPECT_EQ(sock_accept(&c.ss, c.s, NULL), CERR_VAL);
	EXPECT_EQ(sock_accept(&c.vss, c.vs, NULL), CERR_VAL);
	EXPECT_EQ(sock_accept(&c.ss, (void *)-1, &c.p), T_OSOCK_EXPECT(CERR_DESC));
	EXPECT_EQ(sock_accept(&c.vss, (void *)-1, &c.vp), CERR_DESC);
	EXPECT_EQ(sock_accept(&c.ss, c.c, &c.p), T_OSOCK_EXPECT(CERR_STATE));
	EXPECT_EQ(sock_accept(&c.vss, c.vc, &c.vp), CERR_STATE);
	log_set_quiet(0, 0);

	t_sock_conn_close(&c);

	END;
}

TEST(sock_accept_nonblock_empty)
{
	START;

	t_sock_conn_t c = {0};
#if defined(C_LINUX)
	int flags;
#endif
	int vflags;
	t_sock_conn_open(&c);
	t_sock_conn_listen(&c);

#if defined(C_LINUX)
	sock_get_flags(&c.ss, c.s, &flags);
	sock_set_flags(&c.ss, c.s, flags | 04000);
#endif
	sock_get_flags(&c.vss, c.vs, &vflags);
	sock_set_flags(&c.vss, c.vs, vflags | 04000);

	log_set_quiet(0, 1);
	EXPECT_EQ(sock_accept(&c.ss, c.s, &c.p), T_OSOCK_EXPECT(CERR_AGAIN));
	EXPECT_EQ(sock_accept(&c.vss, c.vs, &c.vp), CERR_AGAIN);
	log_set_quiet(0, 0);

	t_sock_conn_close(&c);

	END;
}

TEST(sock_accept_valid)
{
	START;

	t_sock_conn_t c = {0};
	t_sock_conn_open(&c);
	t_sock_conn_listen(&c);
#if defined(C_LINUX)
	sock_connect(&c.ss, c.c, SOCK_FAMILY_UNIX, TEST_SOCK, sizeof(TEST_SOCK));
#endif
	sock_connect(&c.vss, c.vc, SOCK_FAMILY_UNIX, TEST_SOCK, sizeof(TEST_SOCK));

	EXPECT_EQ(sock_accept(&c.ss, c.s, &c.p), T_OSOCK_EXPECT(CERR_OK));
	EXPECT_EQ(sock_accept(&c.vss, c.vs, &c.vp), CERR_OK);

	t_sock_conn_close(&c);

	END;
}

TEST(sock_write_invalid)
{
	START;

	t_sock_conn_t c = {0};
	uint8_t buf[8] = {0};
	size_t n       = 0;
	t_sock_conn_open(&c);
	t_sock_conn_connect(&c);

	EXPECT_EQ(sock_write(NULL, NULL, NULL, 0, NULL), CERR_VAL);
	log_set_quiet(0, 1);
	EXPECT_EQ(sock_write(&c.ss, NULL, buf, 1, &n), CERR_VAL);
	EXPECT_EQ(sock_write(&c.vss, NULL, buf, 1, &n), CERR_VAL);
	EXPECT_EQ(sock_write(&c.ss, c.c, NULL, 1, &n), CERR_VAL);
	EXPECT_EQ(sock_write(&c.vss, c.vc, NULL, 1, &n), CERR_VAL);
	EXPECT_EQ(sock_write(&c.ss, (void *)-1, buf, 1, &n), T_OSOCK_EXPECT(CERR_DESC));
	EXPECT_EQ(sock_write(&c.vss, (void *)-1, buf, 1, &n), CERR_DESC);
	log_set_quiet(0, 0);

	t_sock_conn_close(&c);

	END;
}

TEST(sock_write_unconnected)
{
	START;

	t_sock_pair_t p = {0};
	uint8_t buf[8] = {0};
	size_t n       = 0;

	t_sock_pair_open(&p);

	log_set_quiet(0, 1);
	EXPECT_EQ(sock_write(&p.ss, p.s, buf, 1, &n), T_OSOCK_EXPECT(CERR_CONN));
	EXPECT_EQ(sock_write(&p.vss, p.vs, buf, 1, &n), CERR_CONN);
	log_set_quiet(0, 0);

	t_sock_pair_close(&p);

	END;
}

TEST(sock_write_peer_closed)
{
	START;

	sock_t vss = {0};
	void *s, *c, *p;
	uint8_t buf[8] = {0};
	size_t n       = 0;

	sock_init(&vss, 4, 1, ALLOC_STD);
	sock_open(&vss, SOCK_FAMILY_UNIX, SOCK_TYPE_STREAM, 0, &s);
	sock_open(&vss, SOCK_FAMILY_UNIX, SOCK_TYPE_STREAM, 0, &c);
	sock_bind(&vss, s, SOCK_FAMILY_UNIX, TEST_SOCK, sizeof(TEST_SOCK));
	sock_listen(&vss, s, 1);
	sock_connect(&vss, c, SOCK_FAMILY_UNIX, TEST_SOCK, sizeof(TEST_SOCK));
	sock_accept(&vss, s, &p);
	sock_close(&vss, p);

	log_set_quiet(0, 1);
	EXPECT_EQ(sock_write(&vss, c, buf, 1, &n), CERR_CONN);
	log_set_quiet(0, 0);

	sock_close(&vss, c);
	sock_close(&vss, s);
	sock_free(&vss);

	END;
}

TEST(sock_write_oom)
{
	START;

	sock_t vss = {0};
	void *s, *c, *p;
	uint8_t buf[8] = {0};
	size_t n       = 0;

	sock_init(&vss, 4, 1, ALLOC_STD);
	sock_open(&vss, SOCK_FAMILY_UNIX, SOCK_TYPE_STREAM, 0, &s);
	sock_open(&vss, SOCK_FAMILY_UNIX, SOCK_TYPE_STREAM, 0, &c);
	sock_bind(&vss, s, SOCK_FAMILY_UNIX, TEST_SOCK_WRITE, sizeof(TEST_SOCK_WRITE));
	sock_listen(&vss, s, 1);
	sock_connect(&vss, c, SOCK_FAMILY_UNIX, TEST_SOCK_WRITE, sizeof(TEST_SOCK_WRITE));
	sock_accept(&vss, s, &p);

	mem_oom(1);
	log_set_quiet(0, 1);
	EXPECT_EQ(sock_write(&vss, c, buf, 1, &n), CERR_MEM);
	log_set_quiet(0, 0);
	mem_oom(0);
	EXPECT_EQ(sock_write(&vss, c, buf, 1, &n), CERR_OK);
	mem_oom(1);
	log_set_quiet(0, 1);
	EXPECT_EQ(sock_write(&vss, c, buf, sizeof(buf), &n), CERR_MEM);
	log_set_quiet(0, 0);
	mem_oom(0);

	sock_close(&vss, c);
	sock_close(&vss, s);
	sock_free(&vss);

	END;
}

TEST(sock_write_valid)
{
	START;

	t_sock_conn_t c = {0};
	uint8_t buf[1] = {0x12};
	size_t n       = 0;
	t_sock_conn_open(&c);
	t_sock_conn_connect(&c);

#if defined(C_LINUX)
	EXPECT_EQ(sock_write(&c.ss, c.c, buf, sizeof(buf), &n), CERR_OK);
	EXPECT_EQ(n, sizeof(buf));
#else
	EXPECT_EQ(sock_write(&c.ss, c.c, buf, sizeof(buf), &n), CERR_UNSUPPORTED);
#endif
	n = 0;
	EXPECT_EQ(sock_write(&c.vss, c.vc, buf, sizeof(buf), &n), CERR_OK);
	EXPECT_EQ(n, sizeof(buf));

	t_sock_conn_close(&c);

	END;
}

TEST(sock_read_invalid)
{
	START;

	t_sock_conn_t c = {0};
	uint8_t buf[8] = {0};
	size_t n       = 0;
	t_sock_conn_open(&c);
	t_sock_conn_connect(&c);

	EXPECT_EQ(sock_read(NULL, NULL, NULL, 0, NULL), CERR_VAL);
	log_set_quiet(0, 1);
	EXPECT_EQ(sock_read(&c.ss, NULL, buf, 1, &n), CERR_VAL);
	EXPECT_EQ(sock_read(&c.vss, NULL, buf, 1, &n), CERR_VAL);
	EXPECT_EQ(sock_read(&c.ss, c.p, NULL, 1, &n), CERR_VAL);
	EXPECT_EQ(sock_read(&c.vss, c.vp, NULL, 1, &n), CERR_VAL);
	EXPECT_EQ(sock_read(&c.ss, (void *)-1, buf, 1, &n), T_OSOCK_EXPECT(CERR_DESC));
	EXPECT_EQ(sock_read(&c.vss, (void *)-1, buf, 1, &n), CERR_DESC);
	log_set_quiet(0, 0);

	t_sock_conn_close(&c);

	END;
}

TEST(sock_read_unconnected)
{
	START;

	t_sock_pair_t p = {0};
	uint8_t buf[8] = {0};
	size_t n       = 0;

	t_sock_pair_open(&p);

	log_set_quiet(0, 1);
	EXPECT_EQ(sock_read(&p.ss, p.s, buf, 1, &n), T_OSOCK_EXPECT(CERR_STATE));
	EXPECT_EQ(sock_read(&p.vss, p.vs, buf, 1, &n), CERR_STATE);
	log_set_quiet(0, 0);

	t_sock_pair_close(&p);

	END;
}

TEST(sock_read_nonblock_empty)
{
	START;

	t_sock_conn_t c = {0};
	uint8_t buf[8] = {0};
	size_t n       = 0;
#if defined(C_LINUX)
	int flags;
#endif
	int vflags;
	t_sock_conn_open(&c);
	t_sock_conn_connect(&c);

#if defined(C_LINUX)
	sock_get_flags(&c.ss, c.p, &flags);
	sock_set_flags(&c.ss, c.p, flags | 04000);
#endif
	sock_get_flags(&c.vss, c.vp, &vflags);
	sock_set_flags(&c.vss, c.vp, vflags | 04000);

	log_set_quiet(0, 1);
	EXPECT_EQ(sock_read(&c.ss, c.p, buf, 1, &n), T_OSOCK_EXPECT(CERR_AGAIN));
	EXPECT_EQ(sock_read(&c.vss, c.vp, buf, 1, &n), CERR_AGAIN);
	log_set_quiet(0, 0);

	t_sock_conn_close(&c);

	END;
}

TEST(sock_read_partial)
{
	START;

	t_sock_conn_t c = {0};
	uint8_t buf[8] = {0x12, 0x34, 0x56};
	size_t n       = 0;
	t_sock_conn_open(&c);
	t_sock_conn_connect(&c);

#if defined(C_LINUX)
	sock_write(&c.ss, c.c, buf, 3, &n);
#endif
	sock_write(&c.vss, c.vc, buf, 3, &n);

#if defined(C_LINUX)
	buf[0] = 0;
	buf[1] = 0;
	n      = 0;
	EXPECT_EQ(sock_read(&c.ss, c.p, buf, 2, &n), CERR_OK);
	EXPECT_EQ(n, 2);
	EXPECT_EQ(buf[0], 0x12);
	EXPECT_EQ(buf[1], 0x34);
#else
	EXPECT_EQ(sock_read(&c.ss, c.p, buf, 2, &n), CERR_UNSUPPORTED);
#endif

	buf[0] = 0;
	buf[1] = 0;
	n      = 0;
	EXPECT_EQ(sock_read(&c.vss, c.vp, buf, 2, &n), CERR_OK);
	EXPECT_EQ(n, 2);
	EXPECT_EQ(buf[0], 0x12);
	EXPECT_EQ(buf[1], 0x34);

#if defined(C_LINUX)
	buf[0] = 0;
	n      = 0;
	EXPECT_EQ(sock_read(&c.ss, c.p, buf, 1, &n), CERR_OK);
	EXPECT_EQ(n, 1);
	EXPECT_EQ(buf[0], 0x56);
#endif

	buf[0] = 0;
	n      = 0;
	EXPECT_EQ(sock_read(&c.vss, c.vp, buf, 1, &n), CERR_OK);
	EXPECT_EQ(n, 1);
	EXPECT_EQ(buf[0], 0x56);

	t_sock_conn_close(&c);

	END;
}

STEST(sock)
{
	SSTART;

	RUN(sock_init_free);
	RUN(sock_open_invalid);
	RUN(sock_open_oom);
	RUN(sock_close_invalid);
	RUN(sock_close_valid);
	RUN(sock_setopt_invalid);
	RUN(sock_setopt_valid);
	RUN(sock_get_flags_invalid);
	RUN(sock_get_flags_valid);
	RUN(sock_set_flags_invalid);
	RUN(sock_set_flags_nonblock);
	RUN(sock_bind_invalid);
	RUN(sock_bind_valid);
	RUN(sock_bind_duplicate);
	RUN(sock_bind_oom);
	RUN(sock_listen_invalid);
	RUN(sock_listen_valid);
	RUN(sock_connect_invalid);
	RUN(sock_connect_not_found);
	RUN(sock_connect_not_listening);
	RUN(sock_connect_oom);
	RUN(sock_connect_valid);
	RUN(sock_accept_invalid);
	RUN(sock_accept_nonblock_empty);
	RUN(sock_accept_valid);
	RUN(sock_write_invalid);
	RUN(sock_write_unconnected);
	RUN(sock_write_peer_closed);
	RUN(sock_write_oom);
	RUN(sock_write_valid);
	RUN(sock_read_invalid);
	RUN(sock_read_unconnected);
	RUN(sock_read_nonblock_empty);
	RUN(sock_read_partial);

	SEND;
}
