# Include architecture-specific settings
include ./arch

# Object files
OBJ = life.o real_rand.o plot.o timer.o

# Libraries
LDLIBS = -lpthread -lgnuplot

# Compiler flags
CFLAGS = -g -fopenmp

# Targets
TARGETS = life life_openmp life_pthreads

# Default target
all: $(TARGETS)

# Serial version
life: $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $(OBJ) $(LDLIBS)

# OpenMP version
life_openmp: life_OpenMP.c $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

# Pthreads version
life_pthreads: life_Pthreads.c $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

# Compile source files
%.o: %.c
	$(CC) $(CFLAGS) -c $<

# Clean up
clean:
	$(RM) *.o $(TARGETS) *~ core PI*

# Run serial version
run_serial:
	./life -n 100 -i 200 -p 0.5 -g 2

# Run OpenMP version
run_openmp:
	./life_openmp -n 100 -i 200 -t 4 -g 2

# Run Pthreads version
run_pthreads:
	./life_pthreads -n 100 -i 200 -t 4 -g 2

# Phony targets
.PHONY: all clean run_serial run_openmp run_pthreads