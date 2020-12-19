/*
 ** sigint.c -- grabs SIGINT
 *
 * Read:	http://beej.us/guide/bgipc/html/single/bgipc.html#signals
 * Source:	http://beej.us/guide/bgipc/examples/sigint.c
 *
 * Modified by: hurdleg@algonquincollege.com
 *
 * Usage:
 *  From Momentics IDE, run program; notice PID; enter some text, but don't hit the enter key
 *  At Neutrino prompt, issue the command: kill -s SIGINT <PID>
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

void sigint_handler(int sig);


/*******************************************************************************
 * main( )
 ******************************************************************************/
int main(void) {

	char s[140];
	struct sigaction sa;

	sa.sa_handler = sigint_handler;
	sa.sa_flags = 0; // or SA_RESTART
	sigemptyset(&sa.sa_mask);

	if (sigaction(SIGINT, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}

	printf("My PID is %d\n", getpid());

	printf("Enter a string:  ");

	if (fgets(s, sizeof s, stdin) == NULL)
		perror("fgets");
	else
		printf("You entered: %s\n", s);

	return 0;
}

void sigint_handler(int sig) {
	write(0, "\nAhhh! SIGINT!\n", 14);
}
