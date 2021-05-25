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
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>

#if HAVE_CONFIG_H
# include <config.h>
#endif

bool exitdaemon = false;

void signal_handler(int sig)
{
    switch(sig)
    {
        case SIGINT:
        case SIGTERM:
            exitdaemon = true;
            syslog (LOG_INFO, "signal");
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
    try
    {
        auto logger = spdlog::basic_logger_mt(PACKAGE_NAME, "/tmp/rf24pi.log");
        //spdlog::register_logger(logger);
        spdlog::flush_every(std::chrono::seconds(3));
    }
    catch (const spdlog::spdlog_ex &ex)
    {
        syslog (LOG_ERR, "Log init failed: %s",ex.what());
    }

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

	try
	{
		if(l.start()!=0)
		{
			spdlog::get(PACKAGE_NAME)->error("Failed to start rf24pi!");
			exit(EXIT_FAILURE);
		}
	}
	catch (const std::runtime_error& error)
	{
		spdlog::get(PACKAGE_NAME)->error("not root");
		exit(EXIT_FAILURE);
	}

	syslog (LOG_INFO, "start");

	/* Block */
	while (!exitdaemon) {
	   /* Do some task here ... */
	   sleep(1); /* wait 1 seconds */
	}
	l.stop();

	syslog (LOG_INFO, "stop");

	exit(EXIT_SUCCESS);
}
