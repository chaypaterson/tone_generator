CC=/usr/bin/gcc
ARGS=-lgsl -lm
CODE=audio.c

make : audio.c
	$(CC) $(CODE) $(ARGS)
