NVCC = /usr/local/cuda/bin/nvcc 
NVCC_FLAGS = -g -G -Xcompiler -Wall

all: main.exe 

main.exe: kernel.o
	$(NVCC) $^ -o $@

kernel.o: kernel.cu kernel.h
	$(NVCC) $(NVCC_FLAGS) -c $< -o $@