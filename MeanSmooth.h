#ifndef MEAN_SMOOTH_H
#define MEAN_SMOOTH_H

// smooths sequential data over a window size of 4 using a circular queue
class MeanSmooth {
  public:
    MeanSmooth();
    uint8_t smooth(uint8_t val);
    void clear();

  private:
    static const int mem_len = 4;  // wanted to make this 
    uint8_t memory[mem_len];
    int mem_used;
    int mem_head;
};

#endif
