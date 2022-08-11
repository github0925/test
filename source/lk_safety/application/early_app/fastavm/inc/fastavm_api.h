#ifndef _FASTAVM_API_H_
#define _FASTAVM_API_H_

#define AVM_WIDTH         640
#define AVM_HEIGHT        800

//#define VDSP_ELF_OFFSET     0
//#define VDSP_ELF_SIZE       (2*1024*1024)
//#define VDSP_MAP_TABLE_OFFSET (VDSP_ELF_OFFSET + VDSP_ELF_SIZE)
//#define VDSP_MAP_TABLE_SIZE    (3280012)

typedef struct fastAVM_car{
    uint16_t width;
    uint16_t length;
    uint16_t wheeltrack;
    uint16_t wheelbase;
    uint16_t frontwheeloffset;
} fastAVM_car_t;

typedef struct fastAVM_context{
    int cycles;
} fastAVM_context_t;

typedef struct _fastAVM_{
    uint16_t inputwidth;
    uint16_t inputheight;
    uint32_t denominator;
    uint32_t numerator;
    int elf_size;
    void * pelfbuffer;
    void * pMappingTable;
    fastAVM_context_t status;
} fastAVM_t;

void fastAVM_loadMappingTable(void * pMappingTable);
void fastAVM_loadFastAVM(void * pelfbuffer, int elf_size);
void fastAVM_setCameraResolution(uint16_t width, uint16_t height);
void fastAVM_getCameraResolution(uint16_t * width, uint16_t * height);
void fastAVM_setFPS( uint32_t numerator, uint32_t denominator );
void fastAVM_getFPS( uint32_t * numerator, uint32_t * denominator );
void fastAVM_init(void);
void fastAVM_start(void);
void fastAVM_stop(void);
int fastAVM_update_one_frame(uint8_t * pframebuffer, uint8_t * * pcamerabuffer);
fastAVM_context_t fastAVM_getstatus(void);
fastAVM_car_t fastAVM_getCarPosition(void);

#endif
