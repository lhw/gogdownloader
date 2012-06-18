all:
	clang -o goglogin src/api.c src/http.c src/util.c src/main.c -loauth -lcurl -ljson -g
clean:
	rm goglogin
