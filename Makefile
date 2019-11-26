CC=/usr/bin/gcc
ARGS=-lgsl -lm
CODE=audio.c
STD=-std=c17

make : audio.c
	$(CC) $(CODE) $(ARGS) $(STD)
