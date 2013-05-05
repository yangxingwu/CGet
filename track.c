#include "cget.h"

struct chld *head = NULL;
struct chld *tail = NULL;

int
is_all_done(void)
{
	return ((head == NULL) ? 1 : 0);
}

void
add_to_list(pid_t pid)
{
	struct chld *chld = NULL;

	if ((chld = calloc(1, sizeof(struct chld))) == NULL) {
		DEBUG(ERR, "Alloc memory error: %s\n", strerror(errno));
		return;
	}

	chld->pid = pid;
	chld->next = NULL;

	if (tail == NULL) {
		head = tail = chld;
	} else {
		tail->next = chld;
		tail = chld;
	}
}

void
del_fr_list(pid_t pid)
{
	struct chld *chld = NULL, *prev = NULL;

	for (chld = head; chld != NULL; chld = chld->next) {
		if (chld->pid == pid) {
			if (prev == NULL) {	
				head = head->next;
			} else {
				prev->next = chld->next;
			}
			free(chld);
			break;
		}
		prev = chld;
	}
}

void
track_chld(ACTION action, pid_t pid)
{
	switch (action) {
	case ADD:
		add_to_list(pid);
		break;

	case DEL:
		del_fr_list(pid);
		break;

	default:
		DEBUG(ERR, "Not supported action\n");
		break;
	}

	return;
}
