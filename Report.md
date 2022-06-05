# Report

## Github repo : https://github.com/YU-LIN-LIN/ESL_final_project.git  

## General description or introduction of the problem and your solution
    This project implements selection sort in riscv-vp platform.  
    Also using stratus to do synthesis and get area report.

## Implementation details
![1654440874354](https://user-images.githubusercontent.com/61815140/172056682-0035f764-a5c2-47f9-a603-a5e894a1b8e0.jpg)

    The above figure is the stucture of this sorting system.  
    I store the sequence as an array in global memory in the main function of the software part.  
    Through the IO system and DMA do data transaction with bus connected to the PEs.
    The total length of the sequence is 36 elements which are all integers from 0~255.  
    By setting multiple global addresses for additional PEs amd DMA ports, we can do simulation with more cores  
    and PEs and optimize with parallelism. We also need to do some modification in bootstrap.S file in /sw.

## Additional features of your design and models
    The main function in sw part is used as testbench. To fulfill sorting more than 9 elements sequence,  
    I need to partition the sequence and control the input flow one part by one part, then repeat several times.
    After sorting each part of the sequence in order, I could get a sorted sequence in the end.
    To know more details of how to control the input flow, reference to the 'Additional features ...' part of  
    the "ESL_mid_project" repository.

## Experimental results
![image](https://user-images.githubusercontent.com/61815140/172055798-22127986-9e65-4be2-88a4-4c9343ede28a.png)

    The above figure is the comparison of simulation time and instruction numbers between different cores and PEs between  
    with or without DMA.  
    To be honest, I do not know why the larger number of the core will have more instruction numbers compared to the core0 one.    

![image](https://user-images.githubusercontent.com/61815140/172052134-96668124-39a0-43ba-aadc-5c5c074be86e.png)

    The above table is the synthesis result, including area and lateny under different optimization.
    
    Choosing total sorting latency is 128 cycles and each element in/out equals to 1 cycle, every 9 element sorting needs 146 (9 + 128 + 9) cycles.  
    Then, the cycles I need to sort 36-element sequence between different cores and PEs are :
        1 core + 1 PE     : 4088 cycles  
        2 cores + 2 PEs   : 2336 cycles  
        4 cores + 4 PEs   : 1168 cycles

## Discussions and conclusions
    In the project, I have learned how to implement an application with multicores and DMA using riscv-vp platform.
    I use different numbers of cores and PEs to implement a sorting application.  
    More cores and more PEs can optimize the system with module-level parallelism with less simulation time.  
    However, the instruction numbers in each core could be larger in core1, core2, core3 ... etc.
