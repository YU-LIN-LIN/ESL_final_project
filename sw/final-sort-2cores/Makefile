all_2c1p : main_printf.c bootstrap.S
	riscv32-unknown-elf-gcc main_printf.c bootstrap.S -o main -lm -nostartfiles -march=rv32ima -mabi=ilp32

all_2c2p : main_printf_2c2p.c bootstrap.S
	riscv32-unknown-elf-gcc main_printf_2c2p.c bootstrap.S -o main -lm -nostartfiles -march=rv32ima -mabi=ilp32
	
# 2 cores 1 PE operation
sim_2c1p: all_2c1p
	/home/user/ee6470/docker-images/EE6470/riscv-vp/vp/build/bin/tiny32-mc-final --intercept-syscalls  main

# 2 cores 2 PEs operation
sim: all_2c2p
	/home/user/ee6470/docker-images/EE6470/riscv-vp/vp/build/bin/tiny32-mc-final-2pe --intercept-syscalls  main
	
dump-elf: all
	riscv32-unknown-elf-readelf -a main
	
dump-code: all
	riscv32-unknown-elf-objdump -D main
	
dump-comment: all
	objdump -s --section .comment main
	
clean:
	rm -f main
