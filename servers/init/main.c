#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <signal.h>
#include <lyos/type.h>
#include <lyos/const.h>
#include <lyos/ipc.h>

#define GETTY "/usr/bin/getty"
#define NR_TTY	4

int main(int argc, char * argv[])
{
	int pida = fork();
	if (pida) {
	} else {
		execv("/sbin/ata", NULL);
	}
	pida = fork();
	if (pida) {
	} else {
		execv("/sbin/ext2fs", NULL);
	}

	int fd_stdin  = open("/dev/tty1", O_RDWR);
	int fd_stdout = open("/dev/tty1", O_RDWR);
	int fd_stderr = open("/dev/tty1", O_RDWR);
	
	/* set hostname */
	int fd_hostname = open("/etc/hostname", O_RDONLY);
	if (fd_hostname != -1) {
		char hostname[256];
		memset(hostname, 0, sizeof(hostname));
		int len = read(fd_hostname, hostname, sizeof(hostname));
		printf("Setting hostname to %s\n", hostname);
		sethostname(hostname, len);
		close(fd_hostname);
	}

	mount("/dev/hd1a", "/", "ext2", 0, NULL);

	char * ttylist[NR_TTY] = {"/dev/tty1", "/dev/tty2", "/dev/tty3", "/dev/ttyS0"};
	int i;
	for (i = 0; i < NR_TTY; i++) {
		int pid = fork();
		if (pid) {
			//printf("Parent\n");
		} else {
			close(fd_stdin);
			close(fd_stdout);
			close(fd_stderr);

			open(ttylist[i], O_RDWR);
			open(ttylist[i], O_RDWR);
			open(ttylist[i], O_RDWR);
		
			char * argv[] = {GETTY, ttylist[i], NULL};
			_exit(execv(GETTY, argv));
		}
	}

	while (1) {
		int s;
		wait(&s);
	}

	return 0;
}
