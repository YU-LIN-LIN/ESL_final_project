#ifndef SORTER_DEF_H_
#define SORTER_DEF_H_

#define SEQ_TOT 36                 // total sequence length
#define SEQ_LEN 9
#define MAX 255

const int SORTER_R_ADDR = 0x00000000;
const int SORTER_RESULT_ADDR = 0x00000004;
const int SORTER_CHECK_ADDR = 0x00000008;

union word {
  int sint;
  unsigned int uint;
  unsigned char uc[4];
  unsigned char ans[4];
};


#endif