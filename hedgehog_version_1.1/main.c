#include "hedgehog.h"

asmlinkage long sys_hedgehog(int flag, const char __user *from, const char __user *to) 
{
	int rc = -1;

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
			rc = hedgehog_crypt_file(HASH_MODE, from, to);
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
