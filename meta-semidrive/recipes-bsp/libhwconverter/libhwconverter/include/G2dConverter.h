#ifndef __G2D_CONVERTER_H__
#define __G2D_CONVERTER_H__

#include "HwConverter.h"
class HwBuffer;
class HwConverter;
static G2dConverter *m_instance;
static	std::mutex g2d_lock;
class G2dConverter: public HwConverter {
public:

	~G2dConverter();
	int ConvertSingle(const HwBuffer *input, const HwBuffer *output);
	int BlitSingle(const HwBuffer *input, const HwBuffer *output);

	int Convert(const HwBuffer *inputs[], const HwBuffer *output, const int layer_cnt);
	int Blit(const HwBuffer *inputs[], const HwBuffer* output, const int layer_cnt);
	int addInputHwBuffer(struct g2d_input *post, const HwBuffer *hwbuf, int index);
	int setOutputHwBuffer(struct g2d_input *post, const HwBuffer *hwbuf, bool blit = 0);
	int FillColor(const HwBuffer *hwbuf, uint32_t color_10bit, uint32_t g_alpha);
	int FastCopy(const int dst_fd, const int src_fd, size_t data_size);

public:
	static G2dConverter *getInstance() {
		if (m_instance == NULL) {
			g2d_lock.lock();
			if (m_instance == NULL) {
				m_instance = new G2dConverter;
			}
			g2d_lock.unlock();
		}
		return m_instance;
	}
	inline void Lock() {
		g2d_lock.lock();
	}
	inline void unLock() {
		g2d_lock.unlock();
	}

private:
	G2dConverter();
	int g2dFd;
};
#endif
