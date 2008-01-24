CC = gcc
CFLAGS = -Wall -ansi -pedantic -ggdb -DUSE_ICONV
LDFLAGS = 
BINS = scninfo indexinfo midiinfo idxinfo gdatainfo iffinfo cscninfo \
	smpextract scndecompress musconvert musinfo rbhinfo

OBJS = scninfo.o

all: ${BINS}

scninfo: ${OBJS}
	${CC} -o $@ ${LDFLAGS} ${OBJS}

%info: %info.c
	${CC} -o $@ ${CFLAGS} $<

smpextract: smpextract.c
	${CC} -o $@ ${CFLAGS} smpextract.c

scndecompress: scndecompress.c
	${CC} -o $@ ${CFLAGS} scndecompress.c

musconvert: musconvert.c
	${CC} -o $@ ${CFLAGS} musconvert.c

.c.o:
	${CC} -c -o $@ ${CFLAGS} $<

clean:
	rm -f ${OBJS} ${BINS}

