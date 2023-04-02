CC = cc
CFLAGS = -pedantic -Wall -s -Wno-deprecated-declarations -std=c99 -D_DEFAULT_SOURCE -D_POSIX_C_SOURCE=200809L
LDFLAGS = -lcrypto
SRC = senv.c crypto.c util.c
OBJ = ${SRC:.c=.o}

all: senv

.c.o:
	${CC} -c ${CFLAGS} $<

${OBJ}: crypto.h util.h

senv: ${OBJ}
	${CC} -o $@ ${OBJ} ${LDFLAGS}

debug: ${OBJ}
	${CC} -o $@ $^ -lcrypto -pg
	valgrind --tool=memcheck --leak-check=full -s --expensive-definedness-checks=yes --track-origins=yes ./debug

clean:
	rm -rf *.o

.PHONY: senv debug clean
