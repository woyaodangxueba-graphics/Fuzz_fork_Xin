#cs544 fuzzing project makefile by Li Li

CC = gcc
CFLAGS = -Wall -std=gnu99
#when want to play with pthread add this
LDFLAGS = -pthread 


TARGET = fuzz_draft

#any headers go here
INCLUDES = generator.h

#any .c or .cpp files go here
SOURCE = main.c generator.c

#default is to compile
default: serial

serial: ${SOURCE} ${INCLUDES}
		${CC} -o ${TARGET} ${SOURCE} ${CFLAGS}

pthread: ${SOURCE} ${INCLUDES}
		${CC} -o ${TARGET} ${SOURCE} ${CFLAGS} ${LDFLAGS}
