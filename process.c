#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>

#include "cget.h"

void
send_request(int sockfd, char *host, char *req)
{
	int		sn = 0, n = 0, len = 0;
	char	buf[MAXLINE] = {0}, *p;

	p = buf; len = sizeof(buf);

	if (req == NULL) {
		sn = snprintf(p, len, "GET / HTTP/1.0\r\n"); n += sn; p += sn; len -= sn;
	} else {
		sn = snprintf(p, len, "GET /%s HTTP/1.0\r\n", req); n += sn; p += sn; len -= sn;
	}

	sn = snprintf(p, len, "Host: %s\r\n", host); n += sn; p += sn; len -= sn;
	sn = snprintf(p, len, "Connection: keep-alive\r\n"); n += sn; p += sn; len -= sn;
	sn = snprintf(p, len, "\r\n"); n += sn;

	if (write(sockfd, buf, n) != n) {
		DEBUG(ERR, "Send http request error: %s\n", strerror(errno));
		exit(-1);
	}
}

ssize_t
readline(int fd, void *vptr, size_t maxlen)
{
	ssize_t	n, rc;
	char		c, *ptr;

	ptr = vptr;
	for (n = 1; n < maxlen; n++) {
		if ((rc = read(fd, &c, 1)) == 1) {
			*ptr++ =c;
			if (c == '\n')
				break;
		} else if (rc == 0) {
			*ptr = '\0';
			return 0;
		} else {
			return -1;
		}
	}

	*ptr = '\0';
	return n;
}

void
save_to_file(char *buf, char *path)
{
	FILE *fp;

	if (buf == NULL)
		return;

	if ((fp = fopen(path, "a")) == NULL) {
		DEBUG(ERR, "Can not open file %s: %s\n", path, strerror(errno));
		return;
	}

	fputs(buf, fp);
	fclose(fp);
}

int
recv_reply(int sockfd)
{
	static	int	is_http_data = 0;
	ssize_t	n = 0;
	char		buf[MAXLINE] = {0};
	
	if (is_http_data == 0) { /* read http header */
		if ((n = readline(sockfd, buf, sizeof(buf))) <= 0) {
			DEBUG(ERR, "Receive FIN or RST from server unexpectedly\n");
			exit(-1);
		}

		if (strncmp(buf, "\r\n", n) == 0) {
			is_http_data = 1;
		}
	} else { /* read http data */
		if ((n = read(sockfd, buf, sizeof(buf))) > 0) {
			save_to_file(buf, "home.html");
		}
	}

	DEBUG(ERR, "read: %s\n", buf);
	
	return n;
}

void
process(char *url_p)
{
	int		sockfd;
	char 	*p, *req;
	ssize_t n;
	fd_set	rset;
	struct sockaddr_in dst;
	
	if (url_p == NULL)
		return;

	if ((p = strchr(url_p, '/')) != NULL) {
		*p = '\0';
		req = p + 1;
	} else {
		req = NULL; 
	}

	memset(&dst, 0, sizeof(dst));
	dst.sin_family = AF_INET;
	dst.sin_port = htons(80); /* network byte order */

	if (inet_pton(AF_INET, url_p, & (dst.sin_addr)) != 1) {
		DEBUG(ERR, "%s is not a valid IPV4 address\n");
		exit(-1);
	}

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		DEBUG(ERR, "Create socket error: %s\n", strerror(errno));
		exit(-1);
	}

	if (connect(sockfd, (struct sockaddr *) &dst, sizeof(struct sockaddr_in)) < 0) {
		DEBUG(ERR, "Can not connect to %s: %s\n", url_p, strerror(errno));
		exit(-1);
	}

	if ((req == NULL) || (*req == '\0'))
		send_request(sockfd, url_p, NULL);
	else
		send_request(sockfd, url_p, req);

	while (1) {
		FD_ZERO(&rset);
		FD_SET(sockfd, &rset);

		if (select(sockfd + 1, &rset, NULL, NULL, NULL) <= 0) {
			DEBUG(ERR, "select error: %s\n", strerror(errno));
			return;
		}

		if (FD_ISSET(sockfd, &rset)) {
			n = recv_reply(sockfd);
			if (n == 0) { /* receive FIN */
				return;
			} else if (n == -1) { /* receive RST */
				return;
			}
		}
	}
}
