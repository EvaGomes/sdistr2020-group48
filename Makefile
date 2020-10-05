# Grupo 48
#   Eva Gomes   (37806)
#   João Santos (40335)
#   João Vieira (45677)

SHELL := /bin/sh

EXECS_DIR := binary
HEADERS_DIR := include
OBJS_DIR := object
SOURCES_DIR := source

CC := gcc
CFLAGS := -I./include
CDEBUGFLAGS := -g -Wall


.PHONY: all
all: clean link test

.PHONY: link
link: $(EXECS_DIR)/test_entry.exe $(EXECS_DIR)/test_data.exe
$(EXECS_DIR)/test_entry.exe: $(OBJS_DIR)/test_entry.o $(OBJS_DIR)/entry.o $(OBJS_DIR)/data.o
$(EXECS_DIR)/test_data.exe: $(OBJS_DIR)/test_data.o $(OBJS_DIR)/data.o
$(EXECS_DIR)/test_%.exe:
	$(CC) $(CFLAGS) $^ -o $@
	chmod 777 $@

# compile
$(OBJS_DIR)/%.o: $(SOURCES_DIR)/%.c
	$(CC) $(CFLAGS) -c $^ -o $@

.PHONY: clean
clean:
	rm -rf $(OBJS_DIR)/*.o $(EXECS_DIR)/*

.PHONY: test
test:
	./$(EXECS_DIR)/test_data.exe
	./$(EXECS_DIR)/test_entry.exe
