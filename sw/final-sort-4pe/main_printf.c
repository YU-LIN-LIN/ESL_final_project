/*
    This file is used to do 2 cores with 1 PE.
*/

#include "stdio.h"
#include "string.h"
#include "math.h"
#include "stdint.h"

#include "stdlib.h"

#include "sorter_def.h"
#include "stdbool.h"
// #include "Initiator.h"

// Sorter
static char* const SEQ_START_ADDR0 = (uint32_t * const)(0x73000000);
static char* const SEQ_READ_ADDR0 = (uint32_t * const)(0x73000004);
static char* const SEQ_START_ADDR1 = (uint32_t * const)(0x74000000);
static char* const SEQ_READ_ADDR1 = (uint32_t * const)(0x74000004);
static char* const SEQ_START_ADDR2 = (uint32_t * const)(0x75000000);
static char* const SEQ_READ_ADDR2 = (uint32_t * const)(0x75000004);
static char* const SEQ_START_ADDR3 = (uint32_t * const)(0x76000000);
static char* const SEQ_READ_ADDR3 = (uint32_t * const)(0x76000004);

// DMA 
static volatile uint32_t * const DMA0_SRC_ADDR  = (uint32_t * const)0x70000000;
static volatile uint32_t * const DMA0_DST_ADDR  = (uint32_t * const)0x70000004;
static volatile uint32_t * const DMA0_LEN_ADDR  = (uint32_t * const)0x70000008;
static volatile uint32_t * const DMA0_OP_ADDR   = (uint32_t * const)0x7000000C;
static volatile uint32_t * const DMA0_STAT_ADDR = (uint32_t * const)0x70000010;

static volatile uint32_t * const DMA1_SRC_ADDR  = (uint32_t * const)0x70002000;
static volatile uint32_t * const DMA1_DST_ADDR  = (uint32_t * const)0x70002004;
static volatile uint32_t * const DMA1_LEN_ADDR  = (uint32_t * const)0x70002008;
static volatile uint32_t * const DMA1_OP_ADDR   = (uint32_t * const)0x7000200C;
static volatile uint32_t * const DMA1_STAT_ADDR = (uint32_t * const)0x70002010;

static volatile uint32_t * const DMA2_SRC_ADDR  = (uint32_t * const)0x70004000;
static volatile uint32_t * const DMA2_DST_ADDR  = (uint32_t * const)0x70004004;
static volatile uint32_t * const DMA2_LEN_ADDR  = (uint32_t * const)0x70004008;
static volatile uint32_t * const DMA2_OP_ADDR   = (uint32_t * const)0x7000400C;
static volatile uint32_t * const DMA2_STAT_ADDR = (uint32_t * const)0x70004010;

static volatile uint32_t * const DMA3_SRC_ADDR  = (uint32_t * const)0x70006000;
static volatile uint32_t * const DMA3_DST_ADDR  = (uint32_t * const)0x70006004;
static volatile uint32_t * const DMA3_LEN_ADDR  = (uint32_t * const)0x70006008;
static volatile uint32_t * const DMA3_OP_ADDR   = (uint32_t * const)0x7000600C;
static volatile uint32_t * const DMA3_STAT_ADDR = (uint32_t * const)0x70006010;
static const uint32_t DMA_OP_MEMCPY = 1;

bool _is_using_dma = true;


int sem_init (uint32_t *__sem, uint32_t count) __THROW
{
  *__sem=count;
  return 0;
}

int sem_wait (uint32_t *__sem) __THROW
{
  uint32_t value, success; //RV32A
  __asm__ __volatile__("\
L%=:\n\t\
     lr.w %[value],(%[__sem])            # load reserved\n\t\
     beqz %[value],L%=                   # if zero, try again\n\t\
     addi %[value],%[value],-1           # value --\n\t\
     sc.w %[success],%[value],(%[__sem]) # store conditionally\n\t\
     bnez %[success], L%=                # if the store failed, try again\n\t\
"
    : [value] "=r"(value), [success]"=r"(success)
    : [__sem] "r"(__sem)
    : "memory");
  return 0;
}

int sem_post (uint32_t *__sem) __THROW
{
  uint32_t value, success; //RV32A
  __asm__ __volatile__("\
L%=:\n\t\
     lr.w %[value],(%[__sem])            # load reserved\n\t\
     addi %[value],%[value], 1           # value ++\n\t\
     sc.w %[success],%[value],(%[__sem]) # store conditionally\n\t\
     bnez %[success], L%=                # if the store failed, try again\n\t\
"
    : [value] "=r"(value), [success]"=r"(success)
    : [__sem] "r"(__sem)
    : "memory");
  return 0;
}

int barrier(uint32_t *__sem, uint32_t *__lock, uint32_t *counter, uint32_t thread_count) {
	sem_wait(__lock);
	if (*counter == thread_count - 1) { //all finished
		*counter = 0;
		sem_post(__lock);
		for (int j = 0; j < thread_count - 1; ++j) sem_post(__sem);
	} else {
		(*counter)++;
		sem_post(__lock);
		sem_wait(__sem);
	}
	return 0;
}

//Total number of cores
//static const int PROCESSORS = 2;
#define PROCESSORS 4
//the barrier synchronization objects
uint32_t barrier_counter=0; 
uint32_t barrier_lock; 
uint32_t barrier_sem; 
//the mutex object to control global summation
uint32_t lock;  
//print synchronication semaphore (print in core order)
uint32_t print_sem[PROCESSORS]; 
//global summation variable
float pi_over_4 = 0;






// For sort

unsigned char *source;
unsigned char *target;

unsigned char ele[36] = {35, 23, 12, 0, 20, 7, 3, 33, 27, 26, 8, 9, 13, 30, 5, 16, 11, 32,
                         29, 31, 6, 10, 18, 24, 2, 21, 4, 25, 28, 22, 1, 15, 17, 19, 14, 34};


// int read_seq(string infile_name) {
int read_seq(char * infile_name) {
  FILE *fp_s = NULL; // source file handler
  fp_s = fopen(infile_name, "rb");
  if (fp_s == NULL) {
    printf("fopen %s error\n", infile_name);
    return -1;
  }

  source = (unsigned char *)malloc((size_t)(sizeof(unsigned char) * SEQ_LEN));
  target = (unsigned char *)malloc((size_t)(sizeof(unsigned char) * SEQ_LEN));

  // printf("Read : \n");
  // for (int i = 0;  i < SEQ_TOT; i++) {
  //   fscanf(fp_s, "%d", source + i);
  //   printf("%x %d \n", *(source + i), source + i);
  // } printf("\n");

  fclose(fp_s);
  return 0;
}

// int write_seq(string outfile_name) {
int write_seq(char * outfile_name) {
  FILE *fp_t = NULL;      // target file handler

  fp_t = fopen(outfile_name, "wb");
  if (fp_t == NULL) {
    printf("fopen %s error\n", outfile_name);
    return -1;
  }

  for (int i = 0; i < SEQ_TOT; i++) {
    printf("ele[%d] = %d\n", i, *(ele + i));
    fprintf(fp_t,  "%d\n", *(ele + i));
  }

  return 0;
}

void write_data_to_ACC0(char* ADDR, unsigned char* buffer, int len){
  if(_is_using_dma){  
    // Using DMA 
    *DMA0_SRC_ADDR = (uint32_t)(buffer);
    *DMA0_DST_ADDR = (uint32_t)(ADDR);
    *DMA0_LEN_ADDR = len;
    *DMA0_OP_ADDR  = DMA_OP_MEMCPY;
  }else{
    // Directly Send
    memcpy(ADDR, buffer, sizeof(unsigned char)*len);
  }
}
void read_data_from_ACC0(char* ADDR, unsigned char* buffer, int len){
  if(_is_using_dma){
    // Using DMA 
    *DMA0_SRC_ADDR = (uint32_t)(ADDR);
    *DMA0_DST_ADDR = (uint32_t)(buffer);
    *DMA0_LEN_ADDR = len;
    *DMA0_OP_ADDR  = DMA_OP_MEMCPY;
  }else{
    // Directly Read
    memcpy(buffer, ADDR, sizeof(unsigned char)*len);
  }
}

void write_data_to_ACC1(char* ADDR, unsigned char* buffer, int len){
  if(_is_using_dma){  
    // Using DMA 
    *DMA1_SRC_ADDR = (uint32_t)(buffer);
    *DMA1_DST_ADDR = (uint32_t)(ADDR);
    *DMA1_LEN_ADDR = len;
    *DMA1_OP_ADDR  = DMA_OP_MEMCPY;
  }else{
    // Directly Send
    memcpy(ADDR, buffer, sizeof(unsigned char)*len);
  }
}
void read_data_from_ACC1(char* ADDR, unsigned char* buffer, int len){
  if(_is_using_dma){
    // Using DMA 
    *DMA1_SRC_ADDR = (uint32_t)(ADDR);
    *DMA1_DST_ADDR = (uint32_t)(buffer);
    *DMA1_LEN_ADDR = len;
    *DMA1_OP_ADDR  = DMA_OP_MEMCPY;
  }else{
    // Directly Read
    memcpy(buffer, ADDR, sizeof(unsigned char)*len);
  }
}

void write_data_to_ACC2(char* ADDR, unsigned char* buffer, int len){
  if(_is_using_dma){  
    // Using DMA 
    *DMA2_SRC_ADDR = (uint32_t)(buffer);
    *DMA2_DST_ADDR = (uint32_t)(ADDR);
    *DMA2_LEN_ADDR = len;
    *DMA2_OP_ADDR  = DMA_OP_MEMCPY;
  }else{
    // Directly Send
    memcpy(ADDR, buffer, sizeof(unsigned char)*len);
  }
}
void read_data_from_ACC2(char* ADDR, unsigned char* buffer, int len){
  if(_is_using_dma){
    // Using DMA 
    *DMA2_SRC_ADDR = (uint32_t)(ADDR);
    *DMA2_DST_ADDR = (uint32_t)(buffer);
    *DMA2_LEN_ADDR = len;
    *DMA2_OP_ADDR  = DMA_OP_MEMCPY;
  }else{
    // Directly Read
    memcpy(buffer, ADDR, sizeof(unsigned char)*len);
  }
}

void write_data_to_ACC3(char* ADDR, unsigned char* buffer, int len){
  if(_is_using_dma){  
    // Using DMA 
    *DMA3_SRC_ADDR = (uint32_t)(buffer);
    *DMA3_DST_ADDR = (uint32_t)(ADDR);
    *DMA3_LEN_ADDR = len;
    *DMA3_OP_ADDR  = DMA_OP_MEMCPY;
  }else{
    // Directly Send
    memcpy(ADDR, buffer, sizeof(unsigned char)*len);
  }
}
void read_data_from_ACC3(char* ADDR, unsigned char* buffer, int len){
  if(_is_using_dma){
    // Using DMA 
    *DMA3_SRC_ADDR = (uint32_t)(ADDR);
    *DMA3_DST_ADDR = (uint32_t)(buffer);
    *DMA3_LEN_ADDR = len;
    *DMA3_OP_ADDR  = DMA_OP_MEMCPY;
  }else{
    // Directly Read
    memcpy(buffer, ADDR, sizeof(unsigned char)*len);
  }
}

void fetch_result(int offset, int pe_num);
// unsigned char ele[10] = {10, 9, 4, 3, 5, 8, 1, 2, 7, 6};
// unsigned char ele[9] = {'9', '4', '3', '5', '8', '1', '2', '7', '6'};
void feed_seq(int len_in, int init, int pe_num) {

  unsigned char element;
  unsigned int i;                 // loop index
  unsigned char o_ele[4];

  if (len_in <= 9) {
    for (i = 0; i < SEQ_LEN; i++) {
      if (len_in <= i) {
        o_ele[0] = 255; 
      } else {
        // o_ele[0] = *(source + i);
        o_ele[0] = *(ele + i + init);
      }

      if (pe_num == 0) {
        write_data_to_ACC0(SEQ_START_ADDR0, o_ele, 4);
      } else if (pe_num == 1) {
        write_data_to_ACC1(SEQ_START_ADDR1, o_ele, 4);
      } 
      else if (pe_num == 2) {
        write_data_to_ACC2(SEQ_START_ADDR2, o_ele, 4);
      } else if (pe_num == 3) {
        write_data_to_ACC3(SEQ_START_ADDR3, o_ele, 4);
      }
      
      // if (pe_num == 0) {
      //   printf("pe0 : read %d.\n", o_ele[0]);
      // }
      // printf("write data to ACC over.\n");
    }
    fetch_result(init, pe_num);
  } 
}

void fetch_result(int offset, int pe_num) {
  unsigned int i;       // loop index
  unsigned char result[4];

  for (i = 0; i < SEQ_LEN; i++) {

    if (pe_num == 0) {
      read_data_from_ACC0(SEQ_READ_ADDR0, result, 4);
    } else if (pe_num == 1) {
      read_data_from_ACC1(SEQ_READ_ADDR1, result, 4);
    } else if (pe_num == 2) {
      read_data_from_ACC2(SEQ_READ_ADDR2, result, 4);
    } else if (pe_num == 3) {
      read_data_from_ACC3(SEQ_READ_ADDR3, result, 4);
    }
    *(ele + i + offset) = result[0];
    // if (pe_num == 0) {
    //   printf("pe0 : read %d, %d, %d, %d.\n", result[0], result[1], result[2], result[3]);
    // }
    // cout << "i = " << i << ", result in tb = " << result.uint << endl;
    // if (hart_id == 1) printf("result[0] = %d\n", result[0]);
  }

}






int main(unsigned hart_id) {

	int i;

	/////////////////////////////
	// thread and barrier init //
	/////////////////////////////
	
	if (hart_id == 0) {
		// create a barrier object with a count of PROCESSORS
    sem_init(&barrier_lock, 1);
		sem_init(&barrier_sem, 0); //lock all cores initially
		for(int i=0; i< PROCESSORS; ++i){
			sem_init(&print_sem[i], 0); //lock printing initially
		}
		// Create mutex lock
		sem_init(&lock, 1);
	}

	sem_wait(&lock);
	sem_post(&lock);

	////////////////////////////
	// barrier to synchronize //
	////////////////////////////
	//Wait for all threads to finish
	barrier(&barrier_sem, &barrier_lock, &barrier_counter, PROCESSORS);

  for (int j = 0; j < 4; j++) {
    if (hart_id == 0) {  // Core 0 print first and then others
      feed_seq(9, 0, 0);
    } 
    else if (hart_id == 1) {
        feed_seq(9, 9, 1);
    } else if (hart_id == 2) {  // Core 0 print first and then others
      feed_seq(9, 18, 2);
    } 
    else if (hart_id == 3) {
        feed_seq(9, 27, 3);
    }
    barrier(&barrier_sem, &barrier_lock, &barrier_counter, PROCESSORS);
    if (hart_id == 0) {  // Core 0 print first and then others
      feed_seq(9, 5, 0);
    } 
    else if (hart_id == 1) {
        feed_seq(9, 14, 1);
    } else if (hart_id == 2) {  // Core 0 print first and then others
      feed_seq(9, 23, 2);
    } 
    barrier(&barrier_sem, &barrier_lock, &barrier_counter, PROCESSORS);
  }

  if (hart_id == 0) {
    write_seq("output1.txt");
  }
  barrier(&barrier_sem, &barrier_lock, &barrier_counter, PROCESSORS);

	return 0;
}
