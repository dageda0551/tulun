#ifndef _SYSERR_H
#define _SYSERR_H

#include <syslog.h>
#include <errno.h>
#include <netinet/in.h>


#define SYSERR_MSGSIZE 512
#define log_msg(p, en, ...) log_msg_(p, __FILE__, __func__, __LINE__, en, __VA_ARGS__)
#define log_errpack(p, en, ...) log_errpack_(p, __FILE__, __func__, __LINE__, en, __VA_ARGS__)


extern void init_logger(const char * app, int priority);
extern void close_logger();
extern void change_logger_lvl(int priority);
extern void log_msg_(int pri, char *fn, const char *func, int ln, int en, char *fmt, ...);
extern void log_errpack_(int pri, char *fn, const char *func, int ln, int en, struct sockaddr_in *peer,
		 void *pack, unsigned len, char *fmt, ...);
extern void printfbuffer(uint8_t *buf, uint32_t size);

#endif	/* !_SYSERR_H */
