#include "HwConverter.h"
#include "HwAllocator.h"
#include "G2dConverter.h"
static	std::mutex hw_lock;
static Allocator *m_allocator = nullptr;
HwConverter *HwConverter::getHwConverter()
{
	return G2dConverter::getInstance();
}

Allocator *HwConverter::getAllocator()
{
	hw_lock.lock();
	if (!m_allocator) {
		m_allocator = new GbmAllocator;
	}
	hw_lock.unlock();

	return m_allocator;
}

Allocator *HwConverter::getAllocator(enum ENUM_ALLOCATOR alloc_type)
{
	hw_lock.lock();
	if (!m_allocator) {
		switch (alloc_type) {
			case ENUM_ALLOCATOR_DRM:
				m_allocator = new DrmAllocator;
				break;
			case ENUM_ALLOCATOR_GBM:
				m_allocator = new GbmAllocator;
				break;
			default:
				m_allocator = new GbmAllocator;
			break;
		}
	}
	hw_lock.unlock();

	return m_allocator;
}
