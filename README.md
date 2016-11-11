# Hedgehog

Hedgehog is developed by Jason Xing, aimming to implement encrypt/decrypt/hash function for target file/directory. The reason why i named afterthis program "Hedgehog" is that I want to memorize my beloved and lovely mini-hedgehog that has gone a few mounths ago : (

Why do I design and implement that encrypt/decrypt function?
1. Fisrt and foremost, I'm into linux kernel and I just want to give it a shot (I'm kidding, that's not the reason).
2. eCryptfs is a stackable file system that is used widely in some circumstances, but it has fatal disadvantage that is its advantage - "transparent". (1) User is not quite sure that his files is encrypted and stored in the disk. (2) User does not believe and will not use eCryptfs at very the begining for they do not see the results that file is displayed in encrypted form. (3) User does not have the time or the chance to realize or feel or detect when and how eCryptfs goes wrong, which will cause fatal loss for some companies. (4) Malicious users can copy the file to their flash-drive directly without any protection.
3. The mini-version(version 1.0) is not the ideal one for me, that just marked the time I have embarked on the linux-kernel journey. That code can encrypt/decrypt one file to another file, or it can encrypt/decrypt itself. Until now, i have not fully implemented the key-portion.
4. The rest of my design is going to be put into practice...we will see.
