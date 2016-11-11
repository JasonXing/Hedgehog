#include<unistd.h>
#include<string.h>
#include<fcntl.h> //open()
#include<sys/syscall.h>
#include<stdio.h>

int main()
{
	int rc = 0;
	int fd;
	char *src = "data";
	char *dst = "data";
	//char *dst = "encrypt";

	if (access(src, R_OK)) {
		printf("The file [%s] does not exist, please try again!\n", src);
		rc = 1;
		goto out;
	}
	if (strcmp(src, dst)) {
		if (!access(dst, R_OK)) {
			remove(dst);
			umask(0);
			fd = creat(dst, S_IRWXU);
			close(fd);
		}
		else {
			fd = creat(dst, S_IRWXU);
			close(fd);
		}
	}
	rc = syscall(312, 2, src, dst);
	if (rc) {
		printf("syscall() goes wrong!\n");
	}

out:
	return rc;
}
