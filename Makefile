CC=g++ -std=c++20
CFLAGS=-g -c 

TARGET1=clientlib/libclient.a 
TARGET2=CoordinatorCore/libcoord.a
TARGET3=CoordinatorCore/coordinator.exe
TARGET4=clientlib/pub_main.exe 
TARGET5=clientlib/sub_skt_main.exe 
TARGET6=MasterProcess.exe

TARGET:${TARGET1} ${TARGET2} ${TARGET3} ${TARGET4} ${TARGET5} ${TARGET6} 

COORDINATOR_OBJS=CoordinatorCore/coord_start.o \
										  CoordinatorCore/coord_msg.o \
										  CoordinatorCore/coordDb.o \
										  CoordinatorCore/CoordDistribution.o \
										  Common/cmsgOp.o \
										  Libs/PostgresLibpq/postgresLib.o 

#standard libs
STLIBS=-lpq -lpthread

#Non-standard libs
USERLIBS=-Lclientlib -lclient

${TARGET1}:clientlib/client.o Common/cmsgOp.o
	ar rcs ${TARGET1} clientlib/client.o Common/cmsgOp.o

${TARGET2}:${COORDINATOR_OBJS}
	ar rcs ${TARGET2} ${COORDINATOR_OBJS}

${TARGET3}:CoordinatorCore/main.o ${COORDINATOR_OBJS}
	${CC} -g CoordinatorCore/main.o ${COORDINATOR_OBJS} -o ${TARGET3} ${STLIBS}

${TARGET4}:clientlib/pub_main.o clientlib/pub_example.o clientlib/client.o Common/cmsgOp.o
	${CC} -g clientlib/pub_main.o clientlib/pub_example.o clientlib/client.o Common/cmsgOp.o -o ${TARGET4} ${USERLIBS}

${TARGET5}:clientlib/sub_skt_main.o clientlib/sub_skt_example.o clientlib/client.o Common/cmsgOp.o
	${CC} -g clientlib/sub_skt_main.o clientlib/sub_skt_example.o clientlib/client.o Common/cmsgOp.o -o ${TARGET5} ${USERLIBS}

${TARGET6}:MasterProcess.o Common/cmsgOp.o clientlib/pub_example.o clientlib/sub_skt_example.o CoordinatorCore/coord_start.o
	${CC} -g MasterProcess.o clientlib/pub_example.o clientlib/sub_skt_example.o Common/cmsgOp.o CoordinatorCore/coord_start.o -o ${TARGET6} ${STLIBS} ${USERLIBS}

${TARGET6}:MasterProcess.o ${COORDINATOR_OBJS} clientlib/pub_example.o clientlib/sub_skt_example.o clientlib/sub_cbk_example.o
	${CC} -g MasterProcess.o ${COORDINATOR_OBJS} clientlib/pub_example.o clientlib/sub_skt_example.o clientlib/sub_cbk_example.o -o ${TARGET6} ${STLIBS} ${USERLIBS}

CoordinatorCore/main.o:CoordinatorCore/main.cpp
	${CC} ${CFLAGS} CoordinatorCore/main.cpp -o CoordinatorCore/main.o

CoordinatorCore/coord_start.o:CoordinatorCore/coord_start.cpp
	${CC} ${CFLAGS} CoordinatorCore/coord_start.cpp -o CoordinatorCore/coord_start.o

CoordinatorCore/coord_msg.o:CoordinatorCore/coord_msg.cpp
	${CC} ${CFLAGS} CoordinatorCore/coord_msg.cpp -o CoordinatorCore/coord_msg.o

CoordinatorCore/coordDb.o:CoordinatorCore/coordDb.cpp
	${CC} ${CFLAGS} CoordinatorCore/coordDb.cpp -o CoordinatorCore/coordDb.o

CoordinatorCore/CoordDistribution.o:CoordinatorCore/CoordDistribution.cpp
	${CC} ${CFLAGS} CoordinatorCore/CoordDistribution.cpp -o CoordinatorCore/CoordDistribution.o

Common/cmsgOp.o:Common/cmsgOp.cpp
	${CC} ${CFLAGS} Common/cmsgOp.cpp -o Common/cmsgOp.o

Libs/PostgresLibpq/postgresLib.o:Libs/PostgresLibpq/postgresLib.cpp 
	${CC} ${CFLAGS} Libs/PostgresLibpq/postgresLib.cpp -o Libs/PostgresLibpq/postgresLib.o

clientlib/client.o:clientlib/client.cpp
	${CC} ${CFLAGS} clientlib/client.cpp -o clientlib/client.o

clientlib/pub_example.o:clientlib/pub_example.cpp
	${CC} ${CFLAGS} clientlib/pub_example.cpp -o clientlib/pub_example.o

clientlib/sub_skt_example.o:clientlib/sub_skt_example.cpp
	${CC} ${CFLAGS} clientlib/sub_skt_example.cpp -o clientlib/sub_skt_example.o

clientlib/pub_main.o:clientlib/pub_main.cpp
	${CC} ${CFLAGS} clientlib/pub_main.cpp -o clientlib/pub_main.o

clientlib/sub_skt_main.o:clientlib/sub_skt_main.cpp
	${CC} ${CFLAGS} clientlib/sub_skt_main.cpp -o clientlib/sub_skt_main.o

MasterProcess.o:MasterProcess.cpp
	${CC} ${CFLAGS} MasterProcess.cpp -o MasterProcess.o

clean:
	rm -f *.a
	rm -f *.exe
	rm -rf *.o 
	rm -rf Libs/PostgresLibpq/*.o
	rm -rf Libs/PostgresLibpq/*.a
	rm -rf Common/*.o
	rm -rf clientlib/*.o
	rm -rf clientlib/*.exe
	rm -rf *.o 
	rm -rf *.exe