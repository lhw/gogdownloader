all:
	valac --target-glib=2.32 --vapidir=$(PWD)/vapi --pkg gio-2.0 --pkg posix --thread src/concurrent.vala
	gcc -o goglogin src/gog.h src/gog.c -loauth -lcurl -g
