LIBS:=libcurl oauth json libprotobuf-c
SOURCE:=src/main.c src/api.c src/http.c src/util.c src/serialization.c
PROTOBUF:=state.proto
GENERATED:=src/generated/state.pb-c.c

all: proto goglogin
goglogin: $(SOURCE) $(GENERATED)
	clang -o $@ $^ `pkg-config --cflags --libs $(LIBS)` -g -Wall -Wextra -Wno-unused-function
proto:
	mkdir -p src/generated
	cd src && protoc-c --c_out=generated $(PROTOBUF)
clean:
	rm -f goglogin
	rm -rf src/generated
