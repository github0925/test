#ifndef __HW_CONVERTER_H__
#define __HW_CONVERTER_H__

#include <DrmDisplay.h>
#include <iostream>
#include <mutex>
#include <stdint.h>
#include <string>

#define MAX_DMABUF_PLANES 4
enum ENUM_CONERTER {
	G2D_CONVERTER = 0,
	G2DLITE_CONVERTER,
	DEFAULT_CONVERTER = G2D_CONVERTER,

};

enum ENUM_ALLOCATOR {
	ENUM_ALLOCATOR_GBM = 0,
	ENUM_ALLOCATOR_DRM = 1,
	ENUM_ALLOCATOR_ION,
	ENUM_ALLOCATOR_GRALLOC,
};

enum {
	G2D_OP_CONVERT = 0,
	G2D_OP_BLIT,
};

enum {
	HW_BLEND_PIXEL_NONE,
	HW_BLEND_PIXEL_PREMULTI,
	HW_BLEND_PIXEL_COVERAGE,
};
typedef enum {
    HW_ROTATION_TYPE_NONE = 0,
    HW_ROTATION_TYPE_ROT_90,
    HW_ROTATION_TYPE_HFLIP,
    HW_ROTATION_TYPE_VFLIP,
    HW_ROTATION_TYPE_ROT_180,
    HW_ROTATION_TYPE_ROT_270,
    HW_ROTATION_TYPE_VF_90,
    HW_ROTATION_TYPE_HF_90,
} hw_rotation_type;

struct hw_handle_t
{
    int prime_fd;
    unsigned int gem_handle;

    int magic;

    int width;
    int height;
    int format;
    int usage;
    int data_owner;
    int size;
    int n_planes;
    int fds[MAX_DMABUF_PLANES];
    int offsets[MAX_DMABUF_PLANES];
    int strides[MAX_DMABUF_PLANES];
	uint64_t modifiers[MAX_DMABUF_PLANES];
	void *mapped_vaddrs[MAX_DMABUF_PLANES];
    union
    {
		void *data;
        int64_t __padding;

    } __attribute__((aligned(8)));
};

class Allocator {
public:
	Allocator(){}
	virtual ~Allocator() {}
	virtual int Alloc(struct hw_handle_t *handle) = 0;
	virtual int Free(struct hw_handle_t *handle) = 0;
	virtual int MapBo(struct hw_handle_t *handle) = 0;
	virtual int UnMapBo(struct hw_handle_t *handle) = 0;
	virtual int Import(struct hw_handle_t *handle) = 0;
private:
};

class HwBuffer {
public:
	HwBuffer();
	~HwBuffer();
	HwBuffer(const HwBuffer &obj);
	HwBuffer(struct hw_handle_t *handle);
	HwBuffer(int dma_fd, uint32_t width, uint32_t height, uint32_t stride,
              uint32_t format);
	HwBuffer(uint32_t width, uint32_t height, uint32_t format);

	void MapBo(void);
	void UnMapBo(void);
	HwBuffer& Resize(uint32_t width, uint32_t height);
	HwBuffer& cvtFormat(int new_format);
	HwBuffer& setCrop(const sdm::Rect &crop) {
		source = crop;
		return *this;
	}
	HwBuffer& setDisplay(const sdm::Rect &d) {
		this->display = d;
		return *this;
	}
	HwBuffer& setCrop(int l, int t, int r, int b) {
		source = sdm::Rect(l, t, r, b);
		return *this;
	}
	HwBuffer& setDisplay(int l, int t, int r, int b){
		display = sdm::Rect(l, t, r, b);
		return *this;
	}

public:
	const char *buf_name;
	sdm::Rect source;
	sdm::Rect display;
	uint32_t alpha;
	int blend_mode;
	int zorder;
	int rotation;

	struct hw_handle_t handle;
private:
	Allocator *m_allocator;
};

class G2dConverter;

class HwConverter
{
public:
	virtual ~HwConverter();
	virtual int BlitSingle(const HwBuffer *input, const HwBuffer *output) = 0;
	virtual int ConvertSingle(const HwBuffer *input, const HwBuffer *output) = 0;
	virtual int Convert(const HwBuffer **inputs, const HwBuffer *output, const int layer_cnt) = 0;
	virtual int Blit(const HwBuffer **inputs, const HwBuffer* output, const int layer_cnt) = 0;
	virtual int FillColor(const HwBuffer *hwbuf, uint32_t color_10bit, uint32_t g_alpha) = 0;
	virtual int FastCopy(const int dst_fd, const int src_fd, size_t data_size) = 0;
	virtual int Display(const HwBuffer *input);

	static HwConverter *getHwConverter();
	static Allocator *getAllocator();
	static Allocator *getAllocator(enum ENUM_ALLOCATOR alloc_type);
protected:
	HwConverter();
};

#endif
