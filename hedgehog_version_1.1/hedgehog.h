#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/sched.h>
#include <linux/fcntl.h>
#include <linux/slab.h>
#include <linux/crypto.h>

#define BUFSIZE 4096 
#define FILE_NAME_MAX_SIZE 20 
#define ENCRYPT_MODE 0 
#define DECRYPT_MODE 1
#define HASH_MODE 2
#define MD5_HASH "md5" 
#define HMAC_HASH "hmac(sha256)" 

/*
struct hedgehog_key_tfm {
	struct crypto_cipher *key_tfm;
	size_t key_size;
	struct mutex key_tfm_mutex;
	struct list_head key_tfm_list;
	unsigned char cipher_name[FILE_NAME_MAX_SIZE];
};
*/

//extern struct crypto_cipher *tfm; 
//extern struct kmem_cache *hedgehog_key_tfm_cache; 

void hedgehog_init_key_tfm(void);

int hedgehog_creat_file(const char *pathname);
int hedgehog_open_file(const char *filename, int flags, umode_t mode);
int hedgehog_read_file(unsigned int fd, char *buf, size_t size);
int hedgehog_write_file(unsigned int fd, char *buf, size_t size);
long hedgehog_truncate_file(const char *path, long length);
int hedgehog_copy_file(int fd_src, int fd_dst);
int hedgehog_lseek_file(int fd, int offset, int whence);
int hedgehog_close_file(int fd);

int hedgehog_crypt_file(int mode, const char __user *src, const char __user *dst);
