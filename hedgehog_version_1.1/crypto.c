#include <linux/string.h> 
#include <linux/scatterlist.h> 
#include <linux/highmem.h> 
#include "hedgehog.h"

static int hedgehog_do_crypt(int mode, int fd_src, int fd_dst)
{
	char *buf = NULL;
	struct crypto_cipher *tfm = NULL;
	int blksize;
	char dummy_key[16] = "abcdefg";
	int rc;

	tfm = crypto_alloc_cipher("aes", 0, CRYPTO_ALG_ASYNC);
	if (IS_ERR(tfm)) {
		rc = -1;
		goto out;
	}
	blksize = crypto_cipher_blocksize(tfm);
	crypto_cipher_setkey(tfm, (const unsigned char *)&dummy_key[0], 16);

	buf = kmalloc(blksize, GFP_KERNEL);
	if (buf == NULL) {
		printk(KERN_CRIT "jason: [%s]: kmalloc() goes wrong\n", __FUNCTION__);
		goto out;
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
		}
		else if (mode == DECRYPT_MODE){
			crypto_cipher_decrypt_one(tfm, buf, buf);		
		}
		else {
			printk(KERN_CRIT "jason: mode number is not correct\n");
			goto out;
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
	
out:
	if (tfm) {
		crypto_free_cipher(tfm);	
	}
	if (buf) {
		kfree(buf);
	}
	return rc;
}

static int hedgehog_do_hash(char *mode, int fd_src, int fd_dst)
{
	struct scatterlist sg;
	struct hash_desc desc = {
		.tfm = crypto_alloc_hash(mode, 0, CRYPTO_ALG_ASYNC),
		.flags = CRYPTO_TFM_REQ_MAY_SLEEP
	};
	char *src = NULL, *dst = NULL;
	char *key = "abcdefg";
	int len, hash_size = 16;
	int rc = 0;

	if (!strcmp(mode, HMAC_HASH)) {
		printk(KERN_CRIT "jason: hmac here\n");
		rc = crypto_hash_setkey(desc.tfm, key, 16);
		if (rc) {
			printk(KERN_CRIT "jason: crypto_hash_setkey() goes wrong: [%d]\n", rc);
			goto out;
		}
		hash_size = 32;
	}
	src = kzalloc(4096, GFP_KERNEL);
	if (src == NULL) {
		printk(KERN_CRIT "jason: [%s]: kmalloc() goes wrong\n", __FUNCTION__);
		goto out;
	}
	dst = kzalloc(hash_size, GFP_KERNEL);
	if (dst == NULL) {
		printk(KERN_CRIT "jason: [%s]: kmalloc() goes wrong\n", __FUNCTION__);
		goto out;
	}
	rc = crypto_hash_init(&desc);
	if (rc) {
		printk(KERN_CRIT "jason: crypto_hash_init() goes wrong: [%d]\n", rc);
		goto out;
	}
	while ((len = hedgehog_read_file(fd_src, src, 4096)) > 0) {
		sg_init_one(&sg, (u8 *)src, len);	
		if (IS_ERR(desc.tfm)) {
			rc = PTR_ERR(desc.tfm);
			printk(KERN_CRIT "jason: desc tfm goes wrong: [%d]\n", rc);
			goto out;
		}
		rc = crypto_hash_update(&desc, &sg, len);
		if (rc) {
			printk(KERN_CRIT "jason: desc tfm goes wrong: [%d]\n", rc);
			goto out;
		}
	}
	if (rc < 0) {
		printk(KERN_CRIT "jason: sys_read() goes wrong: [%d]\n", rc);
		goto out;
	}
	rc = crypto_hash_final(&desc, dst);
	if (rc) {
		printk(KERN_CRIT "jason: desc tfm goes wrong: [%d]\n", rc);
		goto out;
	}
	if (hedgehog_write_file(fd_dst, dst, hash_size) != hash_size) {
		printk(KERN_CRIT "jason: sys_write() goes wrong\n");
		rc = -1;
		goto out;
	}

out:
	if (src) {
		kfree(src);
	}
	if (dst) {
		kfree(dst);
	}
	if (desc.tfm) {
		crypto_free_hash(desc.tfm);
	}
	return rc;
}

int hedgehog_crypt_file(int mode, const char __user *src_tmp, const char __user *dst_tmp)
{
	int fd_src, fd_dst;
	char src[FILE_NAME_MAX_SIZE];
	char dst[FILE_NAME_MAX_SIZE];
	int rc = -1;

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
			hedgehog_close_file(fd_src);
			goto out;
		}
	}
	if (mode == HASH_MODE) {
		printk(KERN_CRIT "jason: mode == hash-mode\n");
		//rc = hedgehog_do_hash(MD5_HASH, fd_src, fd_dst);
		rc = hedgehog_do_hash(HMAC_HASH, fd_src, fd_dst);
	} else if (mode == ENCRYPT_MODE || mode == DECRYPT_MODE){
		rc = hedgehog_do_crypt(mode, fd_src, fd_dst);
	} else {
		printk(KERN_CRIT "jason: mode number is not correct\n");
		goto out;
	}
	if (rc < 0) {
		printk(KERN_CRIT "jason: do_crypt() goes wrong: [%d]\n", rc);
	}
	hedgehog_close_file(fd_src);
	if (fd_src != fd_dst)
		hedgehog_close_file(fd_dst);

out:
	return rc;
}
