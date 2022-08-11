int avm_csi_init(uint32_t id);
int avm_csi_config(int id, uint8_t (*pin)[IMG_COUNT][1280*720*2]);
int avm_csi_config_anyRes(int id, uint8_t *pin, uint16_t width, uint16_t height, struct v4l2_fract anyframe_interval);
int avm_csi_start(int id);
int avm_csi_stop(int id);
int avm_csi_close(int id);
void *avm_csi_get_handle(int id);
