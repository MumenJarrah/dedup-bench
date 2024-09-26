#ifndef _AE_CHUNKING_
#define _AE_CHUNKING_

#include <math.h>
#include <iostream>

#include "chunking_common.hpp"
#include "config.hpp"

#include <cstring>

#define DEFAULT_AE_AVG_BLOCK_SIZE 4096

class AE_Chunking : public virtual Chunking_Technique {
   private:
    uint64_t avg_block_size;
    uint64_t window_size;
    uint64_t curr_pos;
    AE_Mode extreme_mode;


    /**
     * @brief finds the next cut point in an array of bytes
     * @param buff: the buff to find the cutpoint in.
     * @param size: the size of the buffer
     * @return: cutpoint position in the buffer 
     */
    uint64_t find_cutpoint(char* buff, uint64_t size) override;

   public:
    /**
     * @brief Default constructor.
     * @return: void
     */
    AE_Chunking();

    /**
     * @brief Constructor with custom config from a config object
     * @param config: the config object
     * @return: void
     */
    AE_Chunking(const Config& config);

    ~AE_Chunking();

};

#endif