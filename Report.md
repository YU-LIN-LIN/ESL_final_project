# Report

## Github repo : https://github.com/YU-LIN-LIN/ESL_final_project.git  

## General description or introduction of the problem and your solution
    This project implements selection sort in riscv-vp platform.  
    Also using stratus to do synthesis and get area report.

## Implementation details
    I store the sequence as an array in global memory in the main function of the software part.  
    The total length of the sequence is 36 elements which are all integers from 0~255.  
    By set multiple global addresses for additional PEs amd DMA ports, we can do simulation with more cores  
    and PEs and optimize with parallelism. We also need to do some modification in bootstrap.S file in /sw.

## Additional features of your design and models
    The main function in sw part is used as testbench. To fulfill sorting more than 9 elements sequence,  
    I need to partition the sequence and control the input flow one part by one part, then repeat several times.
    After sorting each part of the sequence in order, I could have a sorted sequence in the end.
    To know more details of how to control the input flow, reference to the 'Additional features ...' part of  
    the "ESL_mid_project" repository.

## Experimental results
![image](https://user-images.githubusercontent.com/61815140/172017724-776a67c0-b08c-41df-b903-77f011effc7f.png)
![image](https://user-images.githubusercontent.com/61815140/172017758-2aee1b1b-4029-4ede-af2e-6459de0e2945.png)  
    
    The above figure is the comparison of simulation time and instruction numbers between different cores and PEs between  
    with or without DMA. 





## Discussions and conclusions





compare with w/o DMA
