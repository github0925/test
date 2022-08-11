#include "radio.h"

typedef struct _fm_favorite{
    char* fmradio;
    bool bFavorite;
}FM_FAVORITE;

static FM_FAVORITE fm[FM_NUM];
static uint8_t fmSel = FM_NUM - 1;
const char* fmradio[FM_NUM] = {"89.9","90.9","94.0","103.7","107.2"};

void init_fm(void)
{
    for(uint8_t i = 0; i< FM_NUM; i++){
        fm[i].fmradio = (char*)fmradio[i];
        fm[i].bFavorite = false;
    }
}

bool inline get_fav(uint8_t sel)
{
    return fm[sel].bFavorite;
}

void inline set_fav(bool bfav,uint8_t sel)
{
    fm[sel].bFavorite = bfav;
}

char* get_lable(uint8_t sel)
{
    return fm[sel].fmradio;
}
uint8_t inline getsel(void)
{
    return fmSel;
}
void inline setsel(uint8_t sel)
{
    fmSel = sel;
}