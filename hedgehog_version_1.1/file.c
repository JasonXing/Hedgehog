#include <linux/thread_info.h>
#include "hedgehog.h"

/* Return file-descriptor value. If it's negative, then it goes wrong. If positive, then right. */
static int hedgehog_do_creat(const char __user *temp_file, umode_t mode)
{
	sys_umask(0);
	return sys_creat(temp_file, mode);
}

/* Return file-descriptor value. If it's negative, then it goes wrong. If positive, then right. */
int hedgehog_creat_file(const char *pathname)
{
	mm_segment_t old_fs;
	int fd;

	old_fs = get_fs();
	set_fs(KERNEL_DS);

	fd = hedgehog_do_creat(pathname, S_IRWXU);
	if (fd < 0) {
		printk(KERN_CRIT "jason: [%s] goes wrong: [%d]\n", __FUNCTION__, fd);
	}

	set_fs(old_fs);	

	return fd;	
}

static inline int hedgehog_do_open(const char __user *pathname, int flags, umode_t mode)
{
	return sys_open(pathname, O_RDWR | flags, 777);
}

int hedgehog_open_file(const char *filename, int flags, umode_t mode)
{
	mm_segment_t old_fs;
	int fd;

	old_fs = get_fs();
	set_fs(KERNEL_DS);

	fd = hedgehog_do_open(filename, flags, mode);
	if (fd < 0) {
		printk(KERN_CRIT "jason: [%s] goes wrong: [%d].\n", __FUNCTION__, fd);
	}

	set_fs(old_fs);	

	return fd;	
}

static inline int hedgehog_do_read(unsigned int fd, char __user *buf, size_t size)
{
	return sys_read(fd, buf, size);
}

int hedgehog_read_file(unsigned int fd, char *buf, size_t size)
{
	mm_segment_t old_fs;
	int len;

	old_fs = get_fs();
	set_fs(KERNEL_DS);
	len = hedgehog_do_read(fd, buf, size);
	if (len < 0) {
		printk(KERN_CRIT "jason: [%s] goes wrong: [%d].\n", __FUNCTION__, fd);
	}
	set_fs(old_fs);	

	return len;
}

static inline int hedgehog_do_write(unsigned int fd, char __user *buf, size_t size)
{
	return sys_write(fd, buf, size);
}

int hedgehog_write_file(unsigned int fd, char *buf, size_t count)
{
	mm_segment_t old_fs;
	int len;

	old_fs = get_fs();
	set_fs(KERNEL_DS);
	len = hedgehog_do_write(fd, buf, count);
	if (len < 0) {
		printk(KERN_CRIT "jason: [%s] goes wrong: [%d].\n", __FUNCTION__, fd);
	}
	set_fs(old_fs);	
	return len;
}

static inline long hedgehog_do_truncate(const char __user *path, long length)
{
	return sys_truncate(path, length);
}

long truncate_file(const char *path, long length)
{
	mm_segment_t old_fs;
	int rc;

	old_fs = get_fs();
	set_fs(KERNEL_DS);
	rc = hedgehog_do_truncate(path, length);
	if (rc == -1) {
		printk(KERN_CRIT "jason: [%s] goes wrong: [%d].\n", __FUNCTION__, rc);
	}
	set_fs(old_fs);	

	return rc;
}

static int hedgehog_do_copy(int fd_src, int fd_dst)
{
	int len;
	char *buf = NULL;

	buf = kmalloc(BUFSIZE, GFP_KERNEL);
	if (buf == NULL) {
		printk(KERN_CRIT "jason: kmalloc() goes wrong\n");
		goto out;
	}
	while ((len = hedgehog_read_file(fd_src, buf, BUFSIZE)) > 0) {
		if (hedgehog_write_file(fd_dst, buf, len) != len) {
			printk(KERN_CRIT "jason: sys_write() goes wrong: [%d]\n", len);
			goto out;
		}
	}
	if (len < 0) {
		printk(KERN_CRIT "jason: sys_read() goes wrong: [%d]\n", len);
	}

out:
	kfree(buf);
	return 0;
}
int hedgehog_copy_file(int fd_src, int fd_dst)
{
	return hedgehog_do_copy(fd_src, fd_dst);
}

int hedgehog_lseek_file(int fd, int offset, int whence)
{
	return sys_lseek(fd, offset, whence);
}

int hedgehog_close_file(int fd)
{
	return sys_close(fd);
}
