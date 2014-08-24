CC = gcc
CFLAGS = -Wall -g -std=c99

BUILD_DIR = build

all: $(BUILD_DIR) $(BUILD_DIR)/babyc

$(BUILD_DIR):
	mkdir $(BUILD_DIR)

$(BUILD_DIR)/lex.yy.c: babyc_lex.l
	lex -t $< > $@

$(BUILD_DIR)/y.tab.h: babyc_parse.y
	yacc -d $< -o $@

$(BUILD_DIR)/stack.o: stack.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/assembly.o: assembly.c syntax.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/syntax.o: syntax.c list.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/list.o: list.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/environment.o: environment.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/babyc: $(BUILD_DIR)/lex.yy.c $(BUILD_DIR)/y.tab.h $(BUILD_DIR)/syntax.o $(BUILD_DIR)/assembly.o $(BUILD_DIR)/stack.o $(BUILD_DIR)/list.o
	@# FIXME: this is a horrible hack
	cp *.h $(BUILD_DIR)
	$(CC) $(CFLAGS) -Wno-unused-function $(BUILD_DIR)/lex.yy.c $(BUILD_DIR)/syntax.o $(BUILD_DIR)/assembly.o $(BUILD_DIR)/stack.o $(BUILD_DIR)/list.o -o $@

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)

$(BUILD_DIR)/run_tests: run_tests.c $(BUILD_DIR)/babyc
	$(CC) $(CFLAGS) $< -o $@

.PHONY: test
test: $(BUILD_DIR)/run_tests
	@./$^
