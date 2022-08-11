#ifndef SD_MAIN_H
#define SD_MAIN_H
#ifdef __cplusplus
extern "C" {
#endif
#include "container.h"
#include "stdint.h"


#define PNGNUM 35

static const char* png_path[]={"early_app/tlvgl/ic-mileage.png", \
"early_app/tlvgl/ic-seat.png", \
"early_app/tlvgl/ic-temperature-left.png", \
"early_app/tlvgl/ic-temperature-right.png", \
"early_app/tlvgl/ic-auto.png", \
"early_app/tlvgl/ic-cycle-n.png", \
"early_app/tlvgl/ic-front-glass.png", \
"early_app/tlvgl/ic-after-glass.png", \
"early_app/tlvgl/ic-seat-click.png", \
"early_app/tlvgl/ic-temperature-left-click.png", \
"early_app/tlvgl/ic-temperature-right-click.png", \
"early_app/tlvgl/ic-auto-click.png", \
"early_app/tlvgl/ic-cycle-n-click.png", \
"early_app/tlvgl/ic-front-glass-click.png", \
"early_app/tlvgl/ic-after-glass-click.png", \
"early_app/tlvgl/ic-button.png", \
"early_app/tlvgl/ic-button-click.png", \
"early_app/tlvgl/ic-button-temperature-left-click.png", \
"early_app/tlvgl/ic-button-temperature-right-click.png", \
"early_app/tlvgl/ic-line.png", \
"early_app/tlvgl/ic-fm.png", \
"early_app/tlvgl/ic-hvac.png", \
"early_app/tlvgl/ic-shang.png", \
"early_app/tlvgl/ic-play.png", \
"early_app/tlvgl/ic-suspend.png", \
"early_app/tlvgl/ic-xia.png", \
"early_app/tlvgl/ic-sc.png", \
"early_app/tlvgl/ic-fm-click.png", \
"early_app/tlvgl/ic-hvac-click.png", \
"early_app/tlvgl/ic-shang-click.png", \
"early_app/tlvgl/ic-play-click.png", \
"early_app/tlvgl/ic-suspend-click.png", \
"early_app/tlvgl/ic-xia-click.png", \
"early_app/tlvgl/ic-sc-click.png", \
"early_app/tlvgl/ops.png" \
};

void stb_decoder(void);
static inline uint32_t abs(int a) { return (a > 0) ? a : -a; }
#ifdef __cplusplus
} /* extern "C" */
#endif
#endif // SD_MAIN_H
