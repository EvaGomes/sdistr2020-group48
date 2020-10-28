# Grupo 48
#   Eva Gomes   (37806)
#   João Santos (40335)
#   João Vieira (45677)

SHELL := /bin/sh

EXECS_DIR := binary
HEADERS_DIR := include
OBJS_DIR := object
SOURCES_DIR := source

MODULES := serialization tree entry data
TEST_EXECS := $(patsubst %,$(EXECS_DIR)/test_%, $(MODULES))

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
link: $(TEST_EXECS)
$(EXECS_DIR)/test_serialization: $(OBJS_DIR)/test_serialization.o $(OBJS_DIR)/serialization.o \
                                     $(OBJS_DIR)/tree.o $(OBJS_DIR)/entry.o $(OBJS_DIR)/data.o
$(EXECS_DIR)/test_tree: $(OBJS_DIR)/test_tree.o $(OBJS_DIR)/tree.o \
                            $(OBJS_DIR)/entry.o $(OBJS_DIR)/data.o
$(EXECS_DIR)/test_entry: $(OBJS_DIR)/test_entry.o $(OBJS_DIR)/entry.o $(OBJS_DIR)/data.o
$(EXECS_DIR)/test_data: $(OBJS_DIR)/test_data.o $(OBJS_DIR)/data.o
$(TEST_EXECS): %:
	$(CC) $^ $(LINK_CFLAGS) -o $@
	chmod 777 $@

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

.PHONY: test
test:
	@for i in $(TEST_EXECS); \
	do \
		echo "----------------------------------" ; \
		echo "$$i" ; \
		echo "----------------------------------" ; \
		./$$i ; \
		echo "----------------------------------\n" ; \
	done

.PHONY: testMemLeaks
testMemLeaks:
	@for i in $(TEST_EXECS); \
	do \
		echo "----------------------------------" ; \
		echo "$$i" ; \
		echo "----------------------------------" ; \
		valgrind --leak-check=yes ./$$i ; \
		echo "----------------------------------\n" ; \
	done

# --- private tests ---

PRIVATE_TESTS_DIR := tests-private
PRIVATE_TESTS_MODULE := $(PRIVATE_TESTS_DIR)/test_all-private

.PHONY: privateTests
privateTests: cleanPrivateTests linkPrivateTests runPrivateTests

.PHONY: cleanPrivateTests
cleanPrivateTests:
	rm -rf $(PRIVATE_TESTS_DIR)/*.o $(PRIVATE_TESTS_DIR)/*.exe

.PHONY: linkPrivateTests
linkPrivateTests: $(PRIVATE_TESTS_MODULE).exe
$(PRIVATE_TESTS_MODULE).exe: $(PRIVATE_TESTS_MODULE).o \
							 $(OBJS_DIR)/serialization.o \
							 $(OBJS_DIR)/tree.o \
							 $(OBJS_DIR)/entry.o \
							 $(OBJS_DIR)/data.o
	$(CC) $^ $(LINK_CFLAGS) -o $@
	chmod 777 $@
$(PRIVATE_TESTS_MODULE).o: $(PRIVATE_TESTS_MODULE).c
	$(CC) $(COMPILE_CFLAGS) -c $^ -o $@

.PHONY: runPrivateTests
runPrivateTests:
	valgrind --leak-check=yes --quiet ./$(PRIVATE_TESTS_MODULE).exe
