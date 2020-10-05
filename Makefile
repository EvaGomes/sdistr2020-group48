# Grupo 48
#   Eva Gomes   (37806)
#   João Santos (40335)
#   João Vieira (45677)

SHELL = /bin/sh

EXECS_DIR = binary
HEADERS_DIR = include
OBJS_DIR = object
SOURCES_DIR = source


$(EXECS_DIR)/test_data: $(OBJS_DIR)/test_data.o $(OBJS_DIR)/data.o
	gcc -I./include $(OBJS_DIR)/test_data.o $(OBJS_DIR)/data.o -o $(EXECS_DIR)/test_data
	chmod 777 $(EXECS_DIR)/test_data

$(OBJS_DIR)/test_data.o: $(SOURCES_DIR)/test_data.c
	gcc -I./include -c $(SOURCES_DIR)/test_data.c -o $(OBJS_DIR)/test_data.o

$(OBJS_DIR)/data.o: $(SOURCES_DIR)/data.c
	gcc -I./include -c $(SOURCES_DIR)/data.c -o $(OBJS_DIR)/data.o


.PHONY: clean
clean:
	rm -rf $(OBJS_DIR)/*.o $(EXECS_DIR)/*
