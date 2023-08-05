CC = g++
CFLAGS = -o3

main: main.o workflowgraphs.o computeResource.o visualizer.h helper.h globals.h
	$(CC) $(CFLAGS) main.o workflowgraphs.o computeResource.o -o program
main.o: main.cpp visualizer.h helper.h globals.h
	$(CC) $(CFLAGS) -c main.cpp -o main.o
workflowgraphs.o: workflowgraphs.cpp
	$(CC) $(CFLAGS) -c workflowgraphs.cpp -o workflowgraphs.o
computeResource.o: computeResource.cpp
	$(CC) $(CFLAGS) -c computeResource.cpp -o computeResource.o
clean:
	rm -f *.o program
