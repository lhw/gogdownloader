FIND_PACKAGE(ProtoBufC REQUIRED)
INCLUDE_DIRECTORIES(${PROTOBUFC_INCLUDE_DIRS})
SET(LIBS ${LIBS} ${PROTOBUFC_LIBRARIES})

SET(GOG_SRC main.c
				api.c
				http.c
				util.c
				serialization.c
				gog.h
)
ADD_EXECUTABLE(goglogin ${GOG_SRC})
