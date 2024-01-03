CC = gcc
CFLAGS = -I./ -g -O2 -D_REENTRANT -Wall
LIBS = -lncurses

TOOL_OBJS = ./tool/server_component.c ./tool/server_component.h ./tool/snake_client.c ./tool/snake_client.h ./tool/snake_component.h ./tool/snake_server.c ./tool/snake_server.h

all: server_snake client_snake client_snake1

server_snake:	server_snake.c ${TOOL_OBJS}
		${CC} ${CFLAGS} -o $@ $^ ${LIBS}

client_snake:	client_snake.c ${TOOL_OBJS}
		${CC} ${CFLAGS} -o $@ $^ ${LIBS}
		
client_snake1:	client_snake1.c ${TOOL_OBJS}
		${CC} ${CFLAGS} -o $@ $^ ${LIBS}
