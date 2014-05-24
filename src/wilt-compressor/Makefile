OBJS=Compress.o Dictionary.o Main.o RangeCoder.o TempFiles.o

CFLAGS=-std=c99

BetterCompressor: $(OBJS)
	$(CC) -o $@ $(OBJS) -lm

Compress.o: Compress.c Compress.h RangeCoder.h Dictionary.h TempFiles.h
Dictionary.o: Dictionary.c Dictionary.h
Main.o: Main.c Compress.h
RangeCoder.o: RangeCoder.c RangeCoder.h
TempFiles.o: TempFiles.c TempFiles.h
