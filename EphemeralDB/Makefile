all:
	g++ src/hmap.c src/storing.cpp src/buffer.c src/parse.c src/threadpool.cpp src/server.c src/sighan.c test/test2.c -o bin/ephemeraldb -pthread

test0:
	gcc src/threadpool.c test/test0.c -o test/test0
	cd test

test1:
	g++ src/threadpool.cpp test/test1.c -o test/test1 -pthread


test2:
	g++ src/hmap.c src/storing.cpp src/buffer.c src/parse.c src/threadpool.cpp src/server.c src/sighan.c test/test2.c -o test/test2 -pthread


