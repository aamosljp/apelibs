#include "apebuild_internal.h"

#include "unistd.h"
#include <stdio.h>
#include <strings.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>

int ape_dl_fetch(const ApeUrl url, const char *output, size_t size, ApeDownloadOptions *options)
{
	struct hostent *he = gethostbyname(url.domain);
	if (!he) {
		return APE_DL_ERR_DNS;
	}
	int sock;
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		return APE_DL_ERR_NETWORK;
	}

	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(80);
	server_addr.sin_addr = *((struct in_addr *)he->h_addr);
	bzero(&server_addr.sin_zero, 8);
	if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
		return APE_DL_ERR_NETWORK;
	}
	char request[1024];
	snprintf(request, sizeof(request), "GET /%s HTTP/1.1\r\nHost: %s\r\n\r\n", url.path, url.domain);
	send(sock, request, strlen(request), 0);

	char *buf = (char *)APEBUILD_MALLOC(size);
	int len = recv(sock, buf, size, 0);
	buf[len] = '\0';
	char *p = strstr(buf, "\r\n\r\n");
	p = p + 4;
	FILE *fp = fopen(output, "wb+");
	fwrite(p, strlen(p), 1, fp);
	close(sock);
	fclose(fp);
	return APE_DL_OK;
}
