#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>
#include <signal.h>
#include <loop.hpp>

bool exitdaemon = false;

void signal_handler(int sig)
{
    switch(sig)
    {
        case SIGINT:
        case SIGTERM:
            exitdaemon = true;
            break;
    }
}

int main(int argc, char** argv)
{
    struct sigaction newSigAction;
    sigset_t newSigSet;

    /* Set signal mask - signals we want to block */
    sigemptyset(&newSigSet);
    sigaddset(&newSigSet, SIGCHLD);  /* ignore child - i.e. we don't need to wait for it */
    sigaddset(&newSigSet, SIGTSTP);  /* ignore Tty stop signals */
    sigaddset(&newSigSet, SIGTTOU);  /* ignore Tty background writes */
    sigaddset(&newSigSet, SIGTTIN);  /* ignore Tty background reads */
    sigprocmask(SIG_BLOCK, &newSigSet, NULL);   /* Block the above specified signals */

    /* Set up a signal handler */
    newSigAction.sa_handler = signal_handler;
    sigemptyset(&newSigAction.sa_mask);
    newSigAction.sa_flags = 0;

    /* Signals to handle */
    sigaction(SIGHUP, &newSigAction, NULL);     /* catch hangup signal */
    sigaction(SIGTERM, &newSigAction, NULL);    /* catch term signal */
    sigaction(SIGINT, &newSigAction, NULL);     /* catch interrupt signal */

	/* Our process ID and Session ID */
	pid_t pid, sid;

	/* Fork off the parent process */
	pid = fork();
	if (pid < 0) {
			exit(EXIT_FAILURE);
	}
	/* If we got a good PID, then
	   we can exit the parent process. */
	if (pid > 0) {
			exit(EXIT_SUCCESS);
	}

	/* Change the file mode mask */
	umask(0);

	/* Open any logs here */

	/* Create a new SID for the child process */
	sid = setsid();
	if (sid < 0) {
			/* Log the failure */
			exit(EXIT_FAILURE);
	}



	/* Change the current working directory */
	if ((chdir("/")) < 0) {
			/* Log the failure */
			exit(EXIT_FAILURE);
	}

	/* Close out the standard file descriptors */
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);

	/* Daemon-specific initialization goes here */


	Loop l;

	if(l.start()!=0)
	{
		syslog (LOG_ERR, "Failed to start rf24pi!");
		exit(EXIT_FAILURE);
	}

	syslog (LOG_INFO, "Start rf24pi!");

	/* Block */
	while (!exitdaemon) {
	   /* Do some task here ... */
	   sleep(1); /* wait 1 seconds */
	}
	l.stop();

	syslog (LOG_INFO, "Stop rf24pi!");

	exit(EXIT_SUCCESS);
}
