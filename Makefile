all: proto
	clang -o goglogin src/api.c src/http.c src/util.c src/main.c src/generated/state.pb-c.c `pkg-config --cflags --libs libcurl oauth json libprotobuf-c` -g
proto:
	mkdir -p src/generated
	cd src && protoc-c --c_out=generated state.proto
clean:
	rm -f goglogin
	rm -rf src/generated
