// source: https://sdk.upmem.com/2021.3.0/031_DPURuntimeService_Memory.html

#include <mram.h>
#include <stdint.h>
#include <stdio.h>



#define BUFFER_SIZE 256

typedef struct {
    float mass;
    float x, y, z;
    float vx, vy, vz;
} Body_f;


/* Buffer in MRAM. */
uint8_t __mram_noinit mram_array[BUFFER_SIZE];

int main() {
  /* A 256-bytes buffer in WRAM, containing the initial data. */
  __dma_aligned uint8_t input[BUFFER_SIZE];
  /* The other buffer in WRAM, where data are copied back. */
  __dma_aligned uint8_t output[BUFFER_SIZE];

  /* Populate the initial buffer. */
  for (int i = 0; i < BUFFER_SIZE; i++)
    input[i] = i;

  //print the nomber of bytes of datatype uint8_t
  printf("Size of uint8_t: %d\n", sizeof(uint8_t));
  printf("Size of float: %d\n", sizeof(float));
  printf("Size of Body_f: %d\n", sizeof(Body_f));

  // print the size o input 
  printf("Size of input: %d\n", sizeof(input));
  mram_write(input, mram_array, sizeof(input));

  /* Copy back the data. */
  mram_read(mram_array, output, sizeof(output));
  for (int i = 0; i < BUFFER_SIZE; i++)
    if (i != output[i])
      return 1;

  return 0;
}