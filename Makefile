all:
	clang -o goglogin src/api.c src/http.c src/main.c -loauth -lcurl -ljson -g
#	valac --target-glib=2.32 --vapidir=$(PWD)/vapi --pkg gio-2.0 --pkg posix --thread src/concurrent.vala
clean:
	rm goglogin
