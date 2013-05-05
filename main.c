#include <sys/wait.h>

#include "cget.h"

int is_daemon = 0;

int 
main(int argc, char **argv)
{
	int		i;
	int		stat;
	pid_t	pid;

    if (argc < 2) {
        DEBUG(ERR, "Usage: Cget <Url 1> <Url 2> ...\n");
        exit(-1);
    }

	for (i = 1; argv[i] != NULL; i++) {
		if ((pid = fork()) < 0) {
			DEBUG(ERR, "fork error: %s\n", strerror(errno));
			exit(-1);
        } else if (pid > 0) {
			track_chld(ADD, pid);
        } else {
			process(argv[i]);
			exit(0);
        }
    }

    while (!is_all_done()) {
        if ((pid = waitpid(-1, &stat, 0)) > 0) {
            track_chld(DEL, pid);
        }
    }

    exit(0);
}
