/*
 * (C) 2018 by Christian Hesse <mail@eworm.de>
 *
 * This software may be used and distributed according to the terms
 * of the GNU General Public License, incorporated herein by reference.
 */

#include "udp514-journal.h"

int main(int argc, char **argv) {
	int s, rc, n;
	socklen_t len;
	struct sockaddr_in cliAddr, servAddr;
	char buffer[BUFFER_SIZE];
	const int y = 1;

	/* Socket erzeugen */
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("could not open socket");
		return EXIT_FAILURE;
	}

	/* Lokalen Server Port bind(en) */
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAddr.sin_port = htons(LOCAL_SERVER_PORT);
	setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &y, sizeof(int));
	if ((rc = bind(s, (struct sockaddr *) &servAddr, sizeof(servAddr))) < 0) {
		perror("could not bind on port " LOCAL_SERVER_PORT_STR);
		return EXIT_FAILURE;
	}

	/* tell systemd we are ready to go... */
	sd_notify(0, "READY=1\nSTATUS=Listening for syslog input...");

	/* server loop */
	while (1) {
		memset(buffer, 0, BUFFER_SIZE);
		len = sizeof(cliAddr);
		n = recvfrom(s, buffer, BUFFER_SIZE, 0,
			(struct sockaddr *) &cliAddr, &len);
		if (n < 0) {
			perror("could not receive data");
			continue;
		}
	
		/* send to systemd-journald */
		sd_journal_send("MESSAGE=%s: %s", inet_ntoa(cliAddr.sin_addr), buffer,
			"IP_ADDRESS=%s", inet_ntoa(cliAddr.sin_addr),
			NULL);
	}

	return EXIT_SUCCESS;
}