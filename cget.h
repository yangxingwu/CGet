#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define MAXLINE	1500

typedef enum ACTION {
	ADD = 1,
	DEL = 2
} ACTION;

typedef enum LEVEL {
	ERR = 0,
	DBG = 1
} LEVEL;

struct chld {
	pid_t pid;
	struct chld *next;
};

int		is_daemon;

int		is_all_done(void);
void	track_chld(ACTION, pid_t);
void	process(char *);
void	DEBUG(LEVEL, const char *, ...);
