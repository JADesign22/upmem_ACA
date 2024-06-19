
#include <assert.h>
#include <dpu.h>
#include <dpu_log.h>
#include <stdio.h>
#include <stdint.h>

#define DPU_BINARY "helloworld"

int main(){
    struct dpu_set_t set, dpu;
    DPU_ASSERT(dpu_alloc(1, NULL, &set));       //MAKRO DPU_ASSTER -> shows errors
    DPU_ASSERT(dpu_load(set, DPU_BINARY, NULL));
    DPU_ASSERT(dpu_launch(set, DPU_SYNCHRONOUS));
    

    DPU_FOREACH(set, dpu) {
    DPU_ASSERT(dpu_log_read(dpu, stdout));
    }
    
    DPU_ASSERT(dpu_free(set));

    return 0;
}