#include <ae2f/Sys/Sock.h>
#include <stdio.h>
#include <assert.h>
#include <errno.h>

int main(void);
int main(void) {
	ae2fsys_sock_blkmode_t	MEM;
	enum AE2FSYS_SOCK_	RES;
	_ae2fsys_set_sock_blkmode_imp(MEM, STDIN_FILENO, 1, RES);

	char D[3];
	assert(!RES);

	D[0] = 'A';
	D[1] = 0;
	D[2] = 0;

	while(D[0] != 'Q') {
		char V[3];
		if(read(STDIN_FILENO, V, 2) > 0) {
			D[0] = V[0];
		}


		if (!!((errno == EAGAIN)))
		{
			puts(".");
		} else {
			puts(D);
		}
	}

	return 0;
}
