# for cuda
# TODO: merge Rakefile used for src

VPATH = CUDA

.PHONY: cuda cumain cpumain device_prop fmt TAGS help

NVCC_FLAGS = "-O2 -arch=sm_30"
CFLAGS = "-O2"

cuda: cumain device_prop

cumain: idas.cu
	nvcc -o $@ $(NVCC_FLAGS) $<

cpumain: idas_cpu.c
	gcc -o $@ $(CFLAGS) $<

device_prop: device_prop.cu
	nvcc -o

fmt: idas.cu idas_cpu.c
	clang-format -i $^

TAGS:
	ctags -R