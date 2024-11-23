CC=g++
CFLAGS=-g -c

TARGET=coordinator.exe

COORDINATOR_OBJS=coord-main.o \
										  coord_msg.o \
										  coordDb.o \
										  ../Libs/PostgresLibpq/postgresLib.o 

#standard libs
STLIBS=-lpq

#Non-standard libs
USERLIBS=

${TARGET}:main.o ${COORDINATOR_OBJS} ${USERLIBS}
	${CC} -g main.o ${COORDINATOR_OBJS} -o ${TARGET} ${STLIBS}

main.o:main.cpp
	${CC} ${CFLAGS} main.cpp -o main.o
coord-main.o:coord-main.cpp
	${CC} ${CFLAGS} coord-main.cpp -o coord-main.o
coord_msg.o:coord_msg.cpp
	${CC} ${CFLAGS} coord_msg.cpp -o coord_msg.o
coordDb.o:coordDb.cpp
	${CC} ${CFLAGS} coordDb.cpp -o coordDb.o

../Libs/PostgresLibpq/postgresLib.o:../Libs/PostgresLibpq/postgresLib.cpp 
	${CC} ${CFLAGS} ../Libs/PostgresLibpq/postgresLib.cpp -o ../Libs/PostgresLibpq/postgresLib.o
	
clean:
	rm -rf *.o 
	rm -rf ../Libs/PostgresLibpq/*.o
	rm -rf ../Libs/PostgresLibpq/*.a
	rm -rf ${TARGET}