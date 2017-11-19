# --------------------------------------------------------------
# Babyc compiler Makefile
# --------------------------------------------------------------

# CC = gcc -O0 -g -fstack-protector-all -ftrapv
CC = gcc -O3 -fno-stack-protector
CFLAGS = -I./build -Wall -Wextra -std=gnu99

BUILD_DIR = build
OBJ = $(BUILD_DIR)/lex.bb.o $(BUILD_DIR)/bb.tab.o \
      $(BUILD_DIR)/syntax.o $(BUILD_DIR)/list.o \
      $(BUILD_DIR)/assembly.o $(BUILD_DIR)/assembly_binary_op.o \
      $(BUILD_DIR)/assembly_unary_op.o $(BUILD_DIR)/assembly_condition_op.o \
      $(BUILD_DIR)/ast_annotate.o $(BUILD_DIR)/log_error.o \
      $(BUILD_DIR)/main.o

all: $(BUILD_DIR)/babyc

$(BUILD_DIR):
	@mkdir $(BUILD_DIR)

$(BUILD_DIR)/lex.bb.c: babyc_lex.l $(BUILD_DIR)/bb.tab.h
	flex -P bb -t $< > $@

$(BUILD_DIR)/lex.bb.o: $(BUILD_DIR)/lex.bb.c $(BUILD_DIR)/bb.tab.h bb_type.h
	$(CC) $(CFLAGS) -I. -Wno-unused-function -c $< -o $@

$(BUILD_DIR)/bb.tab.c $(BUILD_DIR)/bb.tab.h: babyc_parse.y bb_type.h
	bison -p bb -d $< -o $(BUILD_DIR)/bb.tab.c

$(BUILD_DIR)/bb.tab.o: $(BUILD_DIR)/bb.tab.c syntax.h
	$(CC) $(CFLAGS) -I. -c $< -o $@

$(BUILD_DIR)/assembly.o: assembly.c assembly.h
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/assembly_binary_op.o: assembly_binary_op.c assembly.h
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/assembly_unary_op.o: assembly_unary_op.c assembly.h
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/assembly_condition_op.o: assembly_condition_op.c assembly.h
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/ast_annotate.o: ast_annotate.c syntax.h
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/syntax.o: syntax.c list.h
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/log_error.o: log_error.c log_error.h
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/list.o: list.c list.h
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/main.o: main.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/babyc: $(BUILD_DIR) $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ)

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)

$(BUILD_DIR)/run_tests: run_tests.c $(BUILD_DIR)/babyc
	$(CC) $(CFLAGS) $< -o $@

.PHONY: test
test: $(BUILD_DIR)/run_tests
	@./$^

.PHONY: format
format:
	find -name "*.[ch]" -type f | xargs clang-format -i
