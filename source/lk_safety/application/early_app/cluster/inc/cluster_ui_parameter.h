#define CLUSTER_WIDTH    1920
#define CLUSTER_HEIGHT   720
#define BYTE_PER_PIXEL   4
#define CLUSTER_BUF_SIZE (CLUSTER_WIDTH * CLUSTER_HEIGHT)
#define IMG_BG_SIZE      (CLUSTER_WIDTH * CLUSTER_HEIGHT * 3)

#define NUM_HEIGHT       64
#define NUM_WIDTH        40
#define NUM_STRIDE       440
#define IMG_NUMBER_SIZE  (NUM_WIDTH * NUM_HEIGHT * 11)

#define NEEDLE_HEIGHT    256
#define NEEDLE_WIDTH     256
#define NEEDLE_X_OFFSET  10
#define NEEDLE_Y_OFFSET  245
#define NEEDLE_COUNT     91
#define IMG_NEEDLE_SIZE  (NEEDLE_HEIGHT * NEEDLE_WIDTH * NEEDLE_COUNT)

#define NAV_HEIGHT       720
#define NAV_WIDTH        1120
#define NAV_POS_X        400
#define NAV_POS_Y        0
#define NAV_COLOR        0x80202020
#define NAV_BUF_SIZE     (NAV_HEIGHT * NAV_WIDTH)
#define IMG_NAV_SIZE     (NAV_HEIGHT * NAV_WIDTH)

#define IMG_TOTAL_SIZE   (IMG_BG_SIZE + IMG_NEEDLE_SIZE + IMG_NUMBER_SIZE + IMG_NAV_SIZE)

#define NEEDLE_L_POS_X   403
#define NEEDLE_L_POS_Y   375
#define NEEDLE_R_POS_X   1512
#define NEEDLE_R_POS_Y   375

#define CLEAR_COLOR      0x000000
#define R_COLOR          0xff0000
#define G_COLOR          0x00ff00
#define B_COLOR          0x0000ff

#define NEEDLE_R_COLOR   R_COLOR
#define NEEDLE_L_COLOR   R_COLOR

#define MIN_ANGLE        (270-36)
#define MAX_ANGLE        (270+36)
#define ANGLE_RANGE      288

#define RPM_MAX          8000.0
#define RPM_MIN          0
#define KMPH_MAX         120.0
#define KMPH_MIN         0

#define ANGLE_PER_KMPH   (ANGLE_RANGE/KMPH_MAX)
#define ANGLE_PER_RPM    (ANGLE_RANGE/RPM_MAX)

#define FPS_POS_X        1720
#define FPS_POS_Y        20
#define FPS_COLOR        0xffffff

#define CLUSTER_NAV_EN      1
#define CLUSTER_BG_NAV_ONLY 1
