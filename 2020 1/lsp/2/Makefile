ssu_mntr : main.o ssu_mntr.o lib.o size.o delete.o recover.o  tree.o daemon.o
	gcc main.o ssu_mntr.o lib.o size.o delete.o recover.o tree.o daemon.o  -o ssu_mntr

main.o : main.c
	gcc -c main.c

size.o : size.c ssu_mntr.h
	gcc -c size.c

delete.o : delete.c ssu_mntr.h
	gcc -c delete.c

recover.o : recover.c ssu_mntr.h
	gcc -c recover.c

tree.o : tree.c ssu_mntr.h
	gcc -c tree.c

daemon.o : daemon.c ssu_mntr.h
	gcc -c daemon.c

ssu_mntr.o : ssu_mntr.c ssu_mntr.h
	gcc -c ssu_mntr.c

lib.o : lib.c
	gcc -c lib.c


clean :
	rm *.o
	rm ssu_mntr

