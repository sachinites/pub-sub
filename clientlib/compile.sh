g++ -g -c ../Common/cmsgOp.cpp -o ../Common/cmsgOp.o
g++ -g -c client.cpp -o client.o

g++ -g -c pub_example.cpp -o pub_example.o
g++ -g pub_example.o client.o ../Common/cmsgOp.o -o pub_example.exe

g++ -g -c sub_example.cpp -o sub_example.o
g++ -g sub_example.o client.o ../Common/cmsgOp.o -o sub_example.exe