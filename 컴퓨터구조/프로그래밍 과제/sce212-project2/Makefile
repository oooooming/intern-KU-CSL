CC=gcc
CFLAGS=-g -Wall -O0
OBJS=sce212sim.o util.o proc.o mem.o loader.o
TARGET=sce212sim
SUBMIT = pa2-submission.tar.gz

all: $(TARGET)


$(TARGET): $(OBJS)
	$(CC) -o $@ $(OBJS)

.PHONY: clean test help
clean:
	rm -rf $(OBJS) $(TARGET)

help:
	@echo "The following options are provided with Make\n\t-make:\t\tbuild simulator\n\t-make clean:\tclean the build\n\t-make test:\ttest your simulator"

test: $(TARGET) test_0 test_1 test_2 test_3 test_4 test_5 test_fact test_leaf

test_0: $(TARGET)
	@echo "Testing example0"; \
	./$(TARGET) -m 0x10000000:0x10000003 -n 50 sample_input/example0.o | diff -Naur sample_output/example0 - ;\
	if [ $$? -eq 0 ]; then echo "\tTest seems correct\n"; else echo "\tResults not identical, check the diff output\n"; fi

test_1: $(TARGET)
	@echo "Testing example1"; \
	./$(TARGET) -m 0x10000000:0x10000010 -n 50 sample_input/example1.o | diff -Naur sample_output/example1 - ;\
	if [ $$? -eq 0 ]; then echo "\tTest seems correct\n"; else echo "\tResults not identical, check the diff output\n"; fi

test_2: $(TARGET)
	@echo "Testing example2"; \
	./$(TARGET) -n 50 sample_input/example2.o | diff -Naur sample_output/example2 - ;\
	if [ $$? -eq 0 ]; then echo "\tTest seems correct\n"; else echo "\tResults not identical, check the diff output\n"; fi
test_3: $(TARGET)
	@echo "Testing example3"; \
	./$(TARGET) -n 100 sample_input/example3.o | diff -Naur sample_output/example3 - ;\
	if [ $$? -eq 0 ]; then echo "\tTest seems correct\n"; else echo "\tResults not identical, check the diff output\n"; fi

test_4: $(TARGET)
	@echo "Testing example4"; \
	./$(TARGET) -n 100 sample_input/example4.o | diff -Naur sample_output/example4 - ;\
	if [ $$? -eq 0 ]; then echo "\tTest seems correct\n"; else echo "\tResults not identical, check the diff output\n"; fi

test_5: $(TARGET)
	@echo "Testing example5"; \
	./$(TARGET) -n 100 sample_input/example5.o | diff -Naur sample_output/example5 - ;\
	if [ $$? -eq 0 ]; then echo "\tTest seems correct\n"; else echo "\tResults not identical, check the diff output\n"; fi

test_fact: $(TARGET)
	@echo "Testing fact"; \
	./$(TARGET) -n 100 sample_input/fact.o | diff -Naur sample_output/fact - ;\
	if [ $$? -eq 0 ]; then echo "\tTest seems correct\n"; else echo "\tResults not identical, check the diff output\n"; fi

test_leaf: $(TARGET)
	@echo "Testing leaf_example"; \
	./$(TARGET) -n 100 sample_input/leaf_example.o | diff -Naur sample_output/leaf_example - ;\
	if [ $$? -eq 0 ]; then echo "\tTest seems correct\n"; else echo "\tResults not identical, check the diff output\n"; fi

submission:
	@echo "Generating a compressed file ($(SUBMIT)) including loader.c and proc.c"; \
	tar cvzf $(SUBMIT) loader.c proc.c

