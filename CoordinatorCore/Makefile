CC=g++
CFLAGS=-g -c
TARGET=coordinator.exe
COORDINATOR_OBJS=coord-main.o 
coordinator.exe:main.o ${COORDINATOR_OBJS}

main.o:main.cpp
	${CC} ${CFLAGS} main.cpp -o main.o
coord-main.o:coord-main.cpp
	${CC} ${CFLAGS} coord-main.cpp -o coord-main.o
${TARGET}:
	${CC} -g main.o ${COORDINATOR_OBJS} -o ${TARGET}

clean:
	rm -rf *.o 
	rm -rf ${TARGET}