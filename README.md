# ESL_final_project

### This project's main funtion is sorting. 
### Under riscv-vp platfrm, this repositary will cover two folder, sw and vp, which represents to software part and hardware part respectively. 

## Construct riscv-vp platform  
In /vp folder,  using the following instructions to build riscv-vp platform.  

### Enter the directory /vp
    cd vp

### Create a new folder and enter into it
    mkdir build  
    cd build  

### Compile
    cmake ..

### Install the riscv-vp platform
    make install

### There are major 3 types of design in this project, they are 1 core + 1 PE, 2 cores + 2 PEs and 4 cores + 4 PEs, respectively.  
### We could get into each folder, type the instructions and do the simulation.  

### Enter the directory /sw
    cd sw

### Enter the specific directory depends on the design you choose
1 core + 1 PE  

    cd final-sort-1core
    
2 cores + 2 PEs  

    cd final-sort-2cores
    
4 cores + 4 PEs  

    cd final-sort-4pe
    



