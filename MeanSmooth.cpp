#include "Arduino.h"
#include "MeanSmooth.h"

// Creates a new smoother with window size n
MeanSmooth::MeanSmooth() {
  mem_used = mem_head = 0;
}

// adds a new value to the window, and returns the smoothed value
// if the window is full, the oldest drops off
uint8_t MeanSmooth::smooth(uint8_t val){
  memory[(mem_head+mem_used)%mem_len] = val;
  if (mem_used == mem_len) {
    mem_head = (mem_head+1)%mem_len;
  } else {
    mem_used++;
  }
  unsigned sum = 0;
  for (int i = 0; i < mem_used; i++) {
    sum += memory[(mem_head+i)%mem_len];
  }
  return (uint8_t)(sum/mem_used);
}

// erases the memory window
void MeanSmooth::clear() {
  mem_used = mem_head = 0;
}
