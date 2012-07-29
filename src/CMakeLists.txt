find_package(ProtobufC REQUIRED)
find_package(CURL REQUIRED)
find_package(JSON REQUIRED)
find_package(OAuth REQUIRED)

include_directories(${PROTOBUFC_INCLUDE_DIRS} ${CURL_INCLUDE_DIRS} ${LIBJSON_INCLUDE_DIRS} ${LIBOAUTH_INCLUDE_DIRS})
set(LIBS ${LIBS} ${PROTOBUFC_LIBRARIES} ${LIBJSON_LIBRARIES} ${LIBOAUTH_LIBRARIES} ${CURL_LIBRARIES})

add_definitions(-std=c99 -g -O0)

set(GOG_SRC gog.h
				main.c
				api.c
				http.c
				util.c
				serialization.c
				config.c
)
protoc(PROTO_SRC state.proto config.proto)
add_executable(goglogin ${GOG_SRC} ${PROTO_SRC})
target_link_libraries(goglogin ${LIBS})

install(TARGETS goglogin DESTINATION bin)
install(FILES gog.h DESTINATION include)
