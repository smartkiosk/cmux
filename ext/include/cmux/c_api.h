#ifndef __CMUX__C_API__H__
#define __CMUX__C_API__H__

typedef struct CMUX_CTX CMUX_CTX;

#ifdef __cplusplus
extern "C" {
#endif

CMUX_CTX *cmux_create(char **error);
void cmux_destroy(CMUX_CTX *cmux);

const char *cmux_error(CMUX_CTX *cmux);

int cmux_open(CMUX_CTX *cmux, const char *device);
int cmux_activate(CMUX_CTX *cmux);
int cmux_open_port(CMUX_CTX *cmux, int port, char **device);
int cmux_close_port(CMUX_CTX *cmux, int port);
void cmux_free(void *data);

int cmux_open_device(const char *filename, char **error);

#ifdef __cplusplus
}
#endif

#endif
