CC = gcc
CFLAGS = -Wall -Wextra -g -O0 -std=gnu99 -fstack-protector-all -ftrapv

BUILD_DIR = build

all: $(BUILD_DIR)/babyc

$(BUILD_DIR):
	@mkdir $(BUILD_DIR)

$(BUILD_DIR)/lex.yy.c: babyc_lex.l $(BUILD_DIR)/y.tab.h
	flex -t $< > $@

$(BUILD_DIR)/lex.yy.o: $(BUILD_DIR)/lex.yy.c
	$(CC) $(CFLAGS) -I. -Wno-unused-function -c $< -o $@

$(BUILD_DIR)/y.tab.c $(BUILD_DIR)/y.tab.h: babyc_parse.y
	bison -d $< -o $(BUILD_DIR)/y.tab.c

$(BUILD_DIR)/y.tab.o: $(BUILD_DIR)/y.tab.c syntax.h
	$(CC) $(CFLAGS) -I. -c $< -o $@

$(BUILD_DIR)/assembly.o: assembly.c syntax.h
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/syntax.o: syntax.c list.h
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/list.o: list.c list.h
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/babyc: $(BUILD_DIR) $(BUILD_DIR)/lex.yy.o $(BUILD_DIR)/y.tab.o $(BUILD_DIR)/syntax.o $(BUILD_DIR)/assembly.o $(BUILD_DIR)/list.o main.c
	$(CC) $(CFLAGS) -o $@ main.c $(BUILD_DIR)/lex.yy.o $(BUILD_DIR)/y.tab.o $(BUILD_DIR)/syntax.o $(BUILD_DIR)/assembly.o $(BUILD_DIR)/list.o

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
