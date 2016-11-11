#include <linux/string.h> 
#include <linux/scatterlist.h> 
#include <linux/highmem.h> 
#include "hedgehog.h"

static struct crypto_cipher *tfm;
static int blksize;
//static int filesize;
static char dummy_key[16] = "abcdefg";

void hedgehog_init_key_tfm(void)
{
	tfm = crypto_alloc_cipher("aes", 0, CRYPTO_ALG_ASYNC);
	blksize = crypto_cipher_blocksize(tfm);
	crypto_cipher_setkey(tfm, (const unsigned char *)&dummy_key[0], 16);
}

static int hedgehog_do_crypt(int mode, int fd_src, int fd_dst)
{
	char *buf;
	int rc;

	buf = kmalloc(blksize, GFP_KERNEL);
	if (buf == NULL) {
		printk(KERN_CRIT "jason: [%s]: kmalloc() goes wrong\n", __FUNCTION__);
		return -1;
	}
	while ((rc = hedgehog_read_file(fd_src, buf, blksize)) > 0) {
		if (rc < blksize) {
			memset(buf + rc, 0, blksize - rc);
		}
		if (fd_src == fd_dst) {
			if (hedgehog_lseek_file(fd_src, -rc, SEEK_CUR) == -1) {
				printk(KERN_CRIT "jason: [%s]: lseek_file() goes wrong\n", __FUNCTION__);
				rc = -1;
				goto out;
			}
		}
		if (mode == ENCRYPT_MODE) {
			crypto_cipher_encrypt_one(tfm, buf, buf);		
			//filesize += rc;
		}
		else if (mode == DECRYPT_MODE){
			crypto_cipher_decrypt_one(tfm, buf, buf);		
		}
		if (hedgehog_write_file(fd_dst, buf, blksize) != blksize) {
			printk(KERN_CRIT "jason: sys_write() goes wrong\n");
			rc = -1;
			goto out;
		}
	}
	if (rc < 0) {
		printk(KERN_CRIT "jason: sys_read() goes wrong: [%d]\n", rc);
		goto out;
	}
	//rc = count;
	
out:
	kfree(buf);
	return rc;
}

int hedgehog_crypt_file(int mode, const char __user *src_tmp, const char __user *dst_tmp)
{
	int fd_src, fd_dst;
	char src[FILE_NAME_MAX_SIZE];
	char dst[FILE_NAME_MAX_SIZE];
	int rc;

	strcpy(src, src_tmp);
	strcpy(dst, dst_tmp);

	if (!strcmp(src, dst)) {
		fd_src = fd_dst = hedgehog_open_file(src, 0, 0);
		if (fd_src < 0) {
			rc = fd_src;
			printk(KERN_CRIT "jason: open() goes wrong: [%d]. The file name is [%s]\n", rc, src);
			goto out;
		}
	} 
	else {
		fd_src = hedgehog_open_file(src, 0, 0);
		if (fd_src < 0) {
			rc = fd_src;
			printk(KERN_CRIT "jason: open() goes wrong: [%d]. The file name is [%s]\n", rc, src);
			goto out;
		}
		fd_dst = hedgehog_open_file(dst, 0, 0);
		if (fd_dst < 0) {
			rc = fd_dst;
			printk(KERN_CRIT "jason: open() goes wrong: [%d]. The file name is [%s]\n", rc, dst);
			close_file(fd_src);
			goto out;
		}
	}

	rc = hedgehog_do_crypt(mode, fd_src, fd_dst);
	if (rc < 0) {
		printk(KERN_CRIT "jason: do_crypt() goes wrong: [%d]\n", rc);
	}
	/*
	if (mode == DECRYPT_MODE) {
		if (truncate_file(src, rc) == -1) {
			printk(KERN_CRIT "jason: truncate_file() goes wrong\n");
		}
	}
	*/	
	hedgehog_close_file(fd_src);
	if (fd_src != fd_dst)
		hedgehog_close_file(fd_dst);

out:
	return rc;
}
