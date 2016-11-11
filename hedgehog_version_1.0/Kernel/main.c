#include "hedgehog.h"

static int is_set_tfm;

static void hedgehog_init(void)
{
	hedgehog_init_key_tfm();
	is_set_tfm = 1;
}

asmlinkage long sys_hedgehog(int flag, const char __user *from, const char __user *to) 
{
	int rc = 0;

	if (!is_set_tfm) {
		hedgehog_init();
	}
	switch (flag) {
		case 0: 
			//reserve
			//break;
		case 1: 
			rc = hedgehog_crypt_file(ENCRYPT_MODE, from, to);
			break;
		case 2: 
			rc = hedgehog_crypt_file(DECRYPT_MODE, from, to);
			break;
		case 3: 
			//rc = hash_file(from, to);
			break;
		default:
			printk(KERN_CRIT "jason: wrong: encrypt/decrypt/hash?\n");
			return 1;
	}
	if (rc < 0) {
		printk(KERN_CRIT "jason: encrypt_file() goes wrong\n");
		return 1;
	}

	return 0;
}
