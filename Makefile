# Grupo 48
#   Eva Gomes   (37806)
#   João Santos (40335)
#   João Vieira (45677)

SHELL = /bin/sh

EXECS_DIR = binary
HEADERS_DIR = include
OBJS_DIR = object
SOURCES_DIR = source

CC = gcc
CFLAGS = -I./include


$(EXECS_DIR)/test_data: $(OBJS_DIR)/test_data.o $(OBJS_DIR)/data.o
	$(CC) $(CFLAGS) $^ -o $@
	chmod 777 $(EXECS_DIR)/test_data

$(OBJS_DIR)/%.o: $(SOURCES_DIR)/%.c
	$(CC) $(CFLAGS) -c $^ -o $@


.PHONY: clean
clean:
	rm -rf $(OBJS_DIR)/*.o $(EXECS_DIR)/*
