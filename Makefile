LIBS:=libcurl oauth json libprotobuf-c

all: proto goglogin
goglogin: src/api.c src/http.c src/util.c src/main.c src/generated/state.pb-c.c
	clang -o $@ $^ `pkg-config --cflags --libs $(LIBS)` -g
proto:
	mkdir -p src/generated
	cd src && protoc-c --c_out=generated state.proto
clean:
	rm -f goglogin
	rm -rf src/generated
