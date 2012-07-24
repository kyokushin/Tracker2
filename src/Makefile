CC = gcc

LIB = -L/usr/local/lib -L/usr/lib -L/home/kyokushin/project/MedianFlowTracker2/include/klt -lklt `pkg-config opencv --libs` -lboost_program_options -lboost_filesystem
INCLUDE = -I/home/kyokushin/project/MedianFlowTracker2/include/klt `pkg-config --cflags opencv`
		  
.SUFFIXES: .cpp .o

all: main 

.cpp.o:
	$(CC) -O2 -c $(CFLAGS) $(INCLUDE) $<

main: main.o ImageSequence.o gui.o DelayTracker.o
	$(CC) -O2 -o $@ $^ $(INCLUDE) $(LIB)

clean:
	rm *.o
