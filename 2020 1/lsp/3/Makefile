ssu_crontab : crontab_main.o ssu_crontab.o add.o lib.o daemon.o remove.o rsync_main.o ssu_rsync.o
	gcc crontab_main.o ssu_crontab.o add.o lib.o remove.o -o ssu_crontab
	gcc daemon.o lib.o -o ssu_crond
	gcc rsync_main.o ssu_rsync.o -o ssu_rsync

daemon.o : daemon.o crontab.h
	gcc -c daemon.c

remove.o : remove.c crontab.h
	gcc -c remove.c

crontab_main.o : crontab_main.c crontab.h
	gcc -c crontab_main.c

add.o : add.c crontab.h
	gcc -c add.c

lib.o : lib.c crontab.h
	gcc -c lib.c

ssu_crontab.o : ssu_crontab.c crontab.h
	gcc -c ssu_crontab.c

rsync_main.o : rsync_main.c
	gcc -c rsync_main.c

ssu_rsync.o : ssu_rsync.c rsync.h
	gcc -c ssu_rsync.c

clean :
	rm *.o
	rm ssu_crontab
	rm ssu_crond
	rm ssu_rsync

