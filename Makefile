# Grupo 48
#   Eva Gomes   (37806)
#   João Santos (40335)
#   João Vieira (45677)

SHELL := /bin/sh

EXECS_DIR := binary
HEADERS_DIR := include
OBJS_DIR := object
SOURCES_DIR := source

DEBUG := # assign any value to enable DEBUG_CFLAGS

CC := gcc
DEBUG_CFLAGS := -g -Wall
INCLUDEHEADERS_CFLAGS := -I./include
INCLUDEPROTOBUF_CFLAGS := -I/usr/local/include -L/usr/local/lib -lprotobuf-c

COMPILE_CFLAGS := $(INCLUDEHEADERS_CFLAGS) $(if $(DEBUG), $(DEBUG_CFLAGS),)
LINK_CFLAGS := $(INCLUDEHEADERS_CFLAGS) $(INCLUDEPROTOBUF_CFLAGS) $(if $(DEBUG), $(DEBUG_CFLAGS),)


.PHONY: all
all: clean proto link

.PHONY: link
link:
	@echo "No binaries to build for now"

# compile
$(OBJS_DIR)/%.o: $(SOURCES_DIR)/%.c
	$(CC) $(COMPILE_CFLAGS) -c $^ -o $@

.PHONY: proto
proto: $(SOURCES_DIR)/sdmessage.pb-c.c
$(SOURCES_DIR)/%.pb-c.c: %.proto
	protoc-c sdmessage.proto --c_out=./$(SOURCES_DIR)
	mv $(SOURCES_DIR)/$*.pb-c.h $(HEADERS_DIR)

.PHONY: clean
clean:
	rm -rf $(EXECS_DIR)/* $(HEADERS_DIR)/*.pb-c.h $(OBJS_DIR)/* $(SOURCES_DIR)/*.pb-c.c

# --- tests ---

TESTS_DIR := tests-private
TESTS := data entry tree serialization all-private
TESTS_EXECS := $(TESTS:%=$(TESTS_DIR)/test_%.exe)

.PHONY: privateTests
privateTests: cleanPrivateTests linkPrivateTests runPrivateTests

.PHONY: cleanPrivateTests
cleanPrivateTests:
	rm -rf $(TESTS_DIR)/*.o $(TESTS_DIR)/*.exe

.PHONY: linkPrivateTests
linkPrivateTests: $(TESTS_EXECS)
$(TESTS_DIR)/test_all-private.exe: $(TESTS_DIR)/test_all-private.o \
								   $(OBJS_DIR)/serialization.o $(OBJS_DIR)/sdmessage.pb-c.o \
								   $(OBJS_DIR)/tree.o $(OBJS_DIR)/entry.o $(OBJS_DIR)/data.o
$(TESTS_DIR)/test_serialization.exe: $(TESTS_DIR)/test_serialization.o \
									 $(OBJS_DIR)/serialization.o $(OBJS_DIR)/sdmessage.pb-c.o \
									 $(OBJS_DIR)/tree.o $(OBJS_DIR)/entry.o $(OBJS_DIR)/data.o
$(TESTS_DIR)/test_tree.exe: $(TESTS_DIR)/test_tree.o \
							$(OBJS_DIR)/tree.o $(OBJS_DIR)/entry.o $(OBJS_DIR)/data.o
$(TESTS_DIR)/test_entry.exe: $(TESTS_DIR)/test_entry.o \
							 $(OBJS_DIR)/entry.o $(OBJS_DIR)/data.o
$(TESTS_DIR)/test_data.exe: $(TESTS_DIR)/test_data.o \
							 $(OBJS_DIR)/data.o
$(TESTS_EXECS): %:
	$(CC) $^ $(LINK_CFLAGS) -o $@
	chmod 777 $@

$(TESTS_DIR)/%.o: $(TESTS_DIR)/%.c
	$(CC) $(COMPILE_CFLAGS) -c $^ -o $@

.PHONY: runPrivateTests
runPrivateTests:
	@for i in $(TESTS_EXECS); \
	do \
		echo "----------------------------------" ; \
		echo "$$i" ; \
		echo "----------------------------------" ; \
		valgrind --leak-check=yes $(if $(DEBUG), --track-origins=yes, --quiet) ./$$i ; \
		echo "----------------------------------\n" ; \
	done
