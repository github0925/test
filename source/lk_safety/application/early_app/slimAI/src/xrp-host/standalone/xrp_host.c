/*
 * Copyright (c) 2016 - 2018 Cadence Design Systems Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "string.h"

#include <mbox_hal.h>
#include <err.h>

#define barrier() __asm__ volatile ("dsb" ::: "memory")

typedef uint8_t  __u8;
typedef uint32_t __u32;
typedef uint64_t __u64;

#include "xrp_config.h"
#include "xrp_debug.h"
#include "xrp_host_common.h"
#include "xrp_host_impl.h"
#include "xrp_kernel_dsp_interface.h"
#include "xrp_hw_simple_dsp_interface.h"
#include "xrp_private_alloc.h"
#include "xrp_host.h"

enum {
	XRP_IRQ_NONE,
	XRP_IRQ_LEVEL,
	XRP_IRQ_EDGE,
	XRP_IRQ_EDGE_SW,
	XRP_IRQ_MAX,
};

extern char dt_blob_start[];

struct xrp_comm {
	void *comm_ptr;
	xrp_mutex hw_mutex;
	uint32_t priority;
};

struct xrp_device_description {
	phys_addr_t io_base;
	phys_addr_t comm_base;
	phys_addr_t shared_base;
	phys_addr_t shared_size;
	void *comm_ptr;
	void *shared_ptr;

	unsigned n_queues;
	struct xrp_comm *queue;
	struct xrp_comm **queue_ordered;

	uint32_t device_irq_mode;
	uint32_t device_irq[3];
	uint32_t device_irq_host_offset;
	struct xrp_allocation_pool *shared_pool;
	int sync;

	hal_mb_client_t cl;
	hal_mb_chan_t *mchan;
	event_t vdsp_signal;
};

static struct xrp_device_description xrp_device_description[1]; // only 1 device
static int xrp_device_count = 0;

struct xrp_request {
	struct xrp_queue_item q;
	struct xrp_dsp_cmd dsp_cmd;
	unsigned priority;

	size_t n_buffers;
	size_t in_data_size;
	void *out_data;
	void *out_data_ptr;
	size_t out_data_size;
	struct xrp_buffer_group *buffer_group;
	struct xrp_event *event;

	struct xrp_allocation *in_data_allocation;
	struct xrp_allocation *out_data_allocation;
	struct xrp_allocation *buffer_allocation;
	struct xrp_allocation **user_buffer_allocation;
	struct xrp_dsp_buffer *buffer_ptr;
};

/* Helpers */
static inline void xrp_comm_write32(volatile void *addr, __u32 v)
{
	dprintf(INFO, "%s: 0x%08x, %08x\n", __func__, v2p((const void *)addr), v);
	*(volatile __u32 *)addr = v;
}

static inline __u32 xrp_comm_read32(const volatile void *addr)
{
	return *(volatile __u32 *)addr;
}

static void *xrp_put_tlv(void **addr, uint32_t type, uint32_t length)
{
	struct xrp_dsp_tlv *tlv = *addr;

	xrp_comm_write32(&tlv->type, type);
	xrp_comm_write32(&tlv->length, length);
	*addr = tlv->value + ((length + 3) / 4);
	return tlv->value;
}

static void *xrp_get_tlv(void **addr, uint32_t *type, uint32_t *length)
{
	struct xrp_dsp_tlv *tlv = *addr;

	*type = xrp_comm_read32(&tlv->type);
	*length = xrp_comm_read32(&tlv->length);
	*addr = tlv->value + ((*length + 3) / 4);
	return tlv->value;
}

#if 0
static inline void xrp_send_device_irq(struct xrp_device_description *desc)
{
	void *device_irq = p2v(desc->io_base + desc->device_irq_host_offset);

	switch (desc->device_irq_mode) {
	case XRP_IRQ_EDGE_SW:
		xrp_comm_write32(device_irq, 1 << desc->device_irq[1]);
		/* Wait for interrupt delivery proxy in XTSC to clear */
		while ((xrp_comm_read32(device_irq) &
			(1 << desc->device_irq[1])))
			;
		break;
	case XRP_IRQ_EDGE:
		xrp_comm_write32(device_irq, 0);
		/* fallthrough */
	case XRP_IRQ_LEVEL:
		barrier();
		xrp_comm_write32(device_irq, 1 << desc->device_irq[1]);
		break;
	default:
		break;
	}
}
#else
static void xrp_send_device_irq(struct xrp_device_description *desc)
{
	void *device_irq = p2v(desc->io_base + desc->device_irq_host_offset);
	int ret = -1;

	/* use mailbox if in irq mode */
	if (desc->device_irq_mode) {
		uint32_t msg[2] = {0};
		ret = hal_mb_send_data_dsp(desc->mchan, (u8 *)&msg, 8);
		if (ret != 0) {
			dprintf(CRITICAL, "%s: mb send_data failed %d\n", __func__, ret);
		}
	}
}

int xrp_wait_irq(struct xrp_device_description *desc, int timeout) {
	static int ret = -1;
	if (event_wait_timeout(&(desc->vdsp_signal), timeout) == ERR_TIMED_OUT) {
		dprintf(CRITICAL, "%s: vdsp waiting interrupt timeout!\n", __func__);
		return ret;
	}
	event_unsignal(&(desc->vdsp_signal));
	return 0;
}

#endif
static int synchronize(struct xrp_device_description *desc)
{
	static const int irq_mode[] = {
		[XRP_IRQ_NONE]    = XRP_DSP_SYNC_IRQ_MODE_NONE,
		[XRP_IRQ_LEVEL]   = XRP_DSP_SYNC_IRQ_MODE_LEVEL,
		[XRP_IRQ_EDGE]    = XRP_DSP_SYNC_IRQ_MODE_EDGE,
		[XRP_IRQ_EDGE_SW] = XRP_DSP_SYNC_IRQ_MODE_EDGE,
	};

	struct xrp_dsp_sync_v1 *shared_sync = desc->comm_ptr;
	void *addr = NULL;
	struct xrp_hw_simple_sync_data *hw_sync = NULL;
	__u32 v, v1;

	xrp_comm_write32(&shared_sync->sync, XRP_DSP_SYNC_START);
	//arch_clean_cache_range(&shared_sync->sync, 4);
	arch_clean_invalidate_cache_range((addr_t)(&shared_sync->sync), 4);
	/* use polling before synced */
	do {
		v = xrp_comm_read32(&shared_sync->sync);
		if (v != XRP_DSP_SYNC_START)
			break;
		arch_invalidate_cache_range((addr_t)(&shared_sync->sync), 4);
	} while (1);

	if (v == XRP_DSP_SYNC_DSP_READY_V1) {
		desc->n_queues = 1;
		hw_sync = (struct xrp_hw_simple_sync_data *)&shared_sync->hw_sync_data;
	} else if (v == XRP_DSP_SYNC_DSP_READY_V2) {
		struct xrp_dsp_sync_v2 *shared_sync = desc->comm_ptr;

		addr = shared_sync->hw_sync_data;
		hw_sync = xrp_put_tlv(&addr, XRP_DSP_SYNC_TYPE_HW_SPEC_DATA,
				      sizeof(struct xrp_hw_simple_sync_data));

		if (desc->n_queues > 1) {
			unsigned i;
			uint32_t *priority =
				xrp_put_tlv(&addr, XRP_DSP_SYNC_TYPE_HW_QUEUES,
					    desc->n_queues * 4);

			for (i = 0; i < desc->n_queues; ++i) {
				xrp_comm_write32(priority + i,
						 desc->queue[i].priority);
				if (i) {
					struct xrp_dsp_sync_v1 *queue_sync =
						desc->queue[i].comm_ptr;
					xrp_comm_write32(&queue_sync->sync,
							 XRP_DSP_SYNC_IDLE);
				}
			}
		}

		xrp_put_tlv(&addr, XRP_DSP_SYNC_TYPE_LAST, 0);
	} else {
		dprintf(INFO, "%s: DSP response to XRP_DSP_SYNC_START is not recognized\n",
		       __func__);
		xrp_comm_write32(&shared_sync->sync, XRP_DSP_SYNC_IDLE);
		arch_clean_invalidate_cache_range((addr_t)(&shared_sync->sync), 4);
		return 0;
	}
	xrp_comm_write32(&hw_sync->device_mmio_base,
			 desc->io_base);
	xrp_comm_write32(&hw_sync->host_irq_mode,
			 irq_mode[desc->device_irq_mode]); // use both or neither
	xrp_comm_write32(&hw_sync->device_irq_mode,
			 irq_mode[desc->device_irq_mode]);
	xrp_comm_write32(&hw_sync->device_irq,
			 desc->device_irq[2]);

	arch_clean_cache_range((addr_t)hw_sync, sizeof(struct xrp_hw_simple_sync_data));
	xrp_comm_write32(&shared_sync->sync, XRP_DSP_SYNC_HOST_TO_DSP);
	arch_clean_invalidate_cache_range((addr_t)(&shared_sync->sync), 4);

	do {
		/* sync already invalidated */
		v1 = xrp_comm_read32(&shared_sync->sync);
		if (v1 == XRP_DSP_SYNC_DSP_TO_HOST)
			break;
		arch_invalidate_cache_range((addr_t)(&shared_sync->sync), 4);
	} while (1);

	if (v == XRP_DSP_SYNC_DSP_READY_V2) {
		struct xrp_dsp_sync_v2 *shared_sync = desc->comm_ptr;
		uint32_t type, len;

		addr = shared_sync->hw_sync_data;
		xrp_get_tlv(&addr, &type, &len);
		if (!(type & XRP_DSP_SYNC_TYPE_ACCEPT))
			dprintf(INFO, "%s: HW spec data not recognized by the DSP\n",
			       __func__);
		if (len != sizeof(struct xrp_hw_simple_sync_data)) {
			dprintf(INFO, "HW spec data size modified by the DSP\n");
			return 0;
		}

		if (desc->n_queues > 1) {
			xrp_get_tlv(&addr, &type, &len);

			if (len != desc->n_queues * 4) {
				 dprintf(INFO, "%s: Queue priority size modified by the DSP\n",
					__func__);
				 return 0;
			}
			if (!(type & XRP_DSP_SYNC_TYPE_ACCEPT)) {
				dprintf(INFO, "%s: Queue priority data not recognized by the DSP\n",
				       __func__);
				desc->n_queues = 1;
			}
		}
	}

	xrp_send_device_irq(desc);

	if (desc->device_irq_mode) {
		int res = xrp_wait_irq(desc, 3000);
		if (res) {
			dprintf(CRITICAL,
			        "host IRQ mode is requested, but DSP couldn't deliver IRQ during synchronization\n");
                        return 0;
		}
	}

	xrp_comm_write32(&shared_sync->sync, XRP_DSP_SYNC_IDLE);
	arch_clean_invalidate_cache_range((addr_t)(&shared_sync->sync), 4);
	desc->sync = 1;
	dprintf(INFO, "vdsp sync done\n");
	return 1;
}

static int compare_queue_priority(const void *a, const void *b)
{
	const void * const *ppa = a;
	const void * const *ppb = b;
	const struct xrp_comm *pa = *ppa, *pb = *ppb;

	if (pa->priority == pb->priority)
		return 0;
	else
		return pa->priority < pb->priority ? -1 : 1;
}

static int init_cdns_xrp_common(struct xrp_device_description *description)
{
	uint32_t *priority = NULL;
	const void *p;
	unsigned i;
	int len;

	description->comm_ptr = p2v(description->comm_base);
	if (!description->comm_ptr) {
		dprintf(CRITICAL, "%s: shmem not found for comm area @0x%08x\n",
		       __func__, description->comm_base);
		return 0;
	}

	description->shared_ptr = p2v(description->shared_base);
	if (!description->shared_ptr) {
		dprintf(CRITICAL, "%s: shmem not found for shared area @0x%08x\n",
		       __func__, description->shared_base);
		return 0;
	}
	xrp_init_private_pool(&description->shared_pool,
			      description->shared_base,
			      description->shared_size);

	description->n_queues = 1;

	description->queue = malloc(sizeof(*description->queue) *
				    description->n_queues);
	description->queue_ordered = malloc(sizeof(void *) *
					    description->n_queues);
	for (i = 0; i < description->n_queues; ++i) {
		xrp_mutex_init(&description->queue[i].hw_mutex);
		description->queue[i].comm_ptr =
			description->comm_ptr + XRP_DSP_CMD_STRIDE * i;
		if (priority)
			description->queue[i].priority = priority[i];
		description->queue_ordered[i] = description->queue + i;
	}
	qsort(description->queue_ordered, description->n_queues,
	      sizeof(void *), compare_queue_priority);
	free(priority);
	return 1;
}

static int init_cdns_xrp_v1(struct xrp_device_description *description)
{
	*description = (struct xrp_device_description){
		.comm_base   = XRP_COMM_BASE,
		.shared_base = XRP_SHARED_BASE,
		.shared_size = XRP_SHARED_SIZE,
		.io_base     = XRP_IO_BASE,
	};
	return init_cdns_xrp_common(description);
}

void xrp_callback(hal_mb_client_t cl, void *mssg, u16 len)
{
    event_t* signal = (event_t *)hal_mb_get_user(cl);
    uint32_t ret  = ((uint32_t*)mssg)[0];
    dprintf(INFO, "%s: get vdsp response 0x%lu, len: %d\n", __func__, ret, len);
    event_signal(signal, false);
}

static void *mbox_handle;
static int init_cdns_xrp_hw_simple_common(struct xrp_device_description *description)
{
	uint32_t device_irq             = 0;
	uint32_t device_irq_host_offset = 0;
	uint32_t device_irq_mode;

	hal_mb_cfg_t hal_cfg;

	device_irq_mode = XRP_IRQ_MODE;
	if (device_irq_mode) {
		device_irq = XRP_IRQ_NUM;
		if (device_irq == 0) {
			device_irq_mode = 0;
		} else {
			description->device_irq_mode = 1;
			description->device_irq[0]   = 0;
			description->device_irq[1]   = 0;
			description->device_irq[2]   = XRP_IRQ_NUM;

			device_irq_host_offset = 0;
			description->device_irq_host_offset = device_irq_host_offset;
		}
	}

	if (device_irq_mode) {
		hal_mb_create_handle(&mbox_handle, RES_MB_MB_MEM);

		if (mbox_handle != NULL) {
			hal_mb_init(mbox_handle, &hal_cfg);
		}

		description->cl = hal_mb_get_client();
		if (!description->cl) {
			dprintf(CRITICAL, "%s: get mb cl failed failed\n", __func__);
			return 0;
		}

		description->mchan = hal_mb_request_channel(description->cl, true, xrp_callback, IPCC_RRPOC_VDSP);
		if (!description->mchan) {
			dprintf(CRITICAL, "%s: request mb channel failed\n", __func__);
			hal_mb_put_client(description->cl);
			return 0;
		}

		event_init(&(description->vdsp_signal), false, 0);
		hal_mb_set_user(description->cl, &(description->vdsp_signal));
	}

	return 1;
}


static int init_cdns_xrp_hw_simple_v1(struct xrp_device_description *description)
{
	return init_cdns_xrp_v1(description) &&
		init_cdns_xrp_hw_simple_common(description);
}

static void initialize(void)
{
	xrp_initialize_shmem();

	init_cdns_xrp_hw_simple_v1(xrp_device_description + xrp_device_count);

	++xrp_device_count;
}

/* Device API. */

static void xrp_request_process(struct xrp_queue_item *rq,
				void *context);

struct xrp_device *xrp_open_device(int idx, enum xrp_status *status)
{
	struct xrp_device *device;

	if (!xrp_device_count)
		initialize();

	if (idx < 0 || idx >= xrp_device_count) {
		set_status(status, XRP_STATUS_FAILURE);
		return NULL;
	}
	if (!xrp_device_description[idx].sync &&
	    !synchronize(xrp_device_description + idx)) {
		set_status(status, XRP_STATUS_FAILURE);
		return NULL;
	}

	device = alloc_refcounted(sizeof(*device));
	if (!device) {
		set_status(status, XRP_STATUS_FAILURE);
		return NULL;
	}
	device->impl.description = xrp_device_description + idx;
	set_status(status, XRP_STATUS_SUCCESS);
	return device;
}

void xrp_impl_release_device(struct xrp_device *device)
{
	struct xrp_device_description *desc = device->impl.description;

	if (desc->device_irq_mode) {
		if (desc->mchan)
			hal_mb_free_channel(desc->mchan);

		if (desc->cl)
			hal_mb_put_client(desc->cl);

		event_destroy(&(desc->vdsp_signal));
	}

	if (xrp_device_count)
		xrp_device_count = 0;
}


/* Buffer API. */

void xrp_impl_create_device_buffer(struct xrp_device *device,
				   struct xrp_buffer *buffer,
				   size_t size,
				   enum xrp_status *status)
{
	long rc = xrp_allocate(device->impl.description->shared_pool, size,
			       0x10, &buffer->impl.xrp_allocation);
	if (rc < 0) {
		set_status(status, XRP_STATUS_FAILURE);
		return;
	}
	xrp_retain_device(device);
	buffer->device = device;
	buffer->ptr = p2v(buffer->impl.xrp_allocation->start);
	buffer->size = size;
	set_status(status, XRP_STATUS_SUCCESS);
}

void xrp_impl_release_device_buffer(struct xrp_buffer *buffer)
{
	xrp_free(buffer->impl.xrp_allocation);
	xrp_release_device(buffer->device);
}

/* Queue API. */

void xrp_impl_create_queue(struct xrp_queue *queue,
			   enum xrp_status *status)
{
	xrp_queue_init(&queue->impl.queue, queue->priority,
		       queue->device, xrp_request_process);
	set_status(status, XRP_STATUS_SUCCESS);
}

void xrp_impl_release_queue(struct xrp_queue *queue)
{
	xrp_queue_destroy(&queue->impl.queue);
}

/* Communication API */

static void _xrp_run_command(struct xrp_device *device,
			     struct xrp_request *rq)
{
	struct xrp_device_description *desc = device->impl.description;
	struct xrp_comm *queue = desc->queue;
	struct xrp_dsp_cmd *dsp_cmd;
	size_t i;

	if (desc->n_queues > 1) {
		if (rq->priority >= desc->n_queues)
			rq->priority = desc->n_queues - 1;
		queue = desc->queue_ordered[rq->priority];
	}
	dsp_cmd = queue->comm_ptr;
	xrp_mutex_lock(&queue->hw_mutex);
	dprintf(INFO, "%s: dst:%p src:%p, size:%d\n", __func__, dsp_cmd, &rq->dsp_cmd, sizeof(rq->dsp_cmd));
	memcpy(dsp_cmd, &rq->dsp_cmd, sizeof(rq->dsp_cmd));
	arch_clean_invalidate_cache_range((addr_t)(dsp_cmd), sizeof(rq->dsp_cmd));
	xrp_comm_write32(&dsp_cmd->flags,
			 rq->dsp_cmd.flags | XRP_DSP_CMD_FLAG_REQUEST_VALID);
	arch_clean_invalidate_cache_range((addr_t)(&dsp_cmd->flags), 4);

	xrp_send_device_irq(desc);
	if (desc->device_irq_mode) {
		xrp_wait_irq(desc, 3000);
	}

	do {
		arch_invalidate_cache_range((addr_t)(&dsp_cmd->flags), 4);
	} while ((xrp_comm_read32(&dsp_cmd->flags) &
		  (XRP_DSP_CMD_FLAG_REQUEST_VALID |
		   XRP_DSP_CMD_FLAG_RESPONSE_VALID)) !=
		 (XRP_DSP_CMD_FLAG_REQUEST_VALID |
		  XRP_DSP_CMD_FLAG_RESPONSE_VALID));
	/* dsp_cmd already invalidated */
	memcpy(&rq->dsp_cmd, dsp_cmd, sizeof(rq->dsp_cmd));
	arch_invalidate_cache_range((addr_t)(rq->out_data_ptr), rq->out_data_size);
	memcpy(rq->out_data, rq->out_data_ptr, rq->out_data_size);
	xrp_comm_write32(&dsp_cmd->flags, 0);
	arch_clean_invalidate_cache_range((addr_t)(&dsp_cmd->flags), 4);
	xrp_mutex_unlock(&queue->hw_mutex);

	if (rq->in_data_size > XRP_DSP_CMD_INLINE_DATA_SIZE) {
		xrp_free(rq->in_data_allocation);
	}
	if (rq->out_data_size > XRP_DSP_CMD_INLINE_DATA_SIZE) {
		xrp_free(rq->out_data_allocation);
	}

	if (rq->buffer_group)
		xrp_mutex_lock(&rq->buffer_group->mutex);

	for (i = 0; i < rq->n_buffers; ++i) {
		phys_addr_t addr;

		if (rq->buffer_group->buffer[i].buffer->type != XRP_BUFFER_TYPE_DEVICE) {
			if (!xrp_translatable(rq->buffer_group->buffer[i].buffer->ptr)) { // shadow copied
				if (rq->buffer_ptr[i].flags & XRP_DSP_BUFFER_FLAG_WRITE) {
					addr = rq->user_buffer_allocation[i]->start;
					arch_invalidate_cache_range((addr_t)(p2v(addr)),
					                            rq->buffer_group->buffer[i].buffer->size);
					memcpy(rq->buffer_group->buffer[i].buffer->ptr, p2v(addr),
					       rq->buffer_group->buffer[i].buffer->size);
				}
				xrp_free(rq->user_buffer_allocation[i]);
			} else {
				if (rq->buffer_ptr[i].flags & XRP_DSP_BUFFER_FLAG_WRITE) {
					addr = (phys_addr_t)p2v((phys_addr_t)(rq->buffer_group->buffer[i].buffer->ptr));
					arch_invalidate_cache_range((addr_t)addr,
					                            rq->buffer_group->buffer[i].buffer->size);
				}
			}
		} else { // XRP_BUFFER_TYPE_DEVICE
			/* need do inval cache anyway, as device buffer range is still cacheable */
			addr = (phys_addr_t)p2v((phys_addr_t)(rq->buffer_group->buffer[i].buffer->ptr));
			arch_invalidate_cache_range((addr_t)addr, rq->buffer_group->buffer[i].buffer->size);
		}
	}

	if (rq->n_buffers > XRP_DSP_CMD_INLINE_BUFFER_COUNT) {
		xrp_free(rq->buffer_allocation);
	}

	if (rq->buffer_group) {
		xrp_mutex_unlock(&rq->buffer_group->mutex);
		xrp_release_buffer_group(rq->buffer_group);
	}

	if (rq->event) {
		struct xrp_event *event = rq->event;
		xrp_cond_lock(&event->impl.cond);
		if (rq->dsp_cmd.flags & XRP_DSP_CMD_FLAG_RESPONSE_DELIVERY_FAIL)
			event->status = XRP_STATUS_FAILURE;
		else
			event->status = XRP_STATUS_SUCCESS;
		xrp_cond_broadcast(&event->impl.cond);
		xrp_cond_unlock(&event->impl.cond);
		xrp_release_event(event);
	}
	free(rq->user_buffer_allocation);
	free(rq);
}

static void xrp_request_process(struct xrp_queue_item *rq,
				void *context)
{
	_xrp_run_command(context, (struct xrp_request *)rq);
}

void xrp_enqueue_command(struct xrp_queue *queue,
			 const void *in_data, size_t in_data_size,
			 void *out_data, size_t out_data_size,
			 struct xrp_buffer_group *buffer_group,
			 struct xrp_event **evt,
			 enum xrp_status *status)
{
	struct xrp_device *device = queue->device;
	size_t n_buffers;
	size_t i;
	struct xrp_request *rq = malloc(sizeof(*rq));
	struct xrp_dsp_cmd *dsp_cmd = &rq->dsp_cmd;
	void *in_data_ptr;

	rq->priority = queue->priority;
	rq->in_data_size = in_data_size;
	rq->out_data = out_data;
	rq->out_data_size = out_data_size;
	rq->buffer_group = buffer_group;
	if (buffer_group)
		xrp_retain_buffer_group(buffer_group);

	if (in_data_size > XRP_DSP_CMD_INLINE_DATA_SIZE) {
		long rc = xrp_allocate(device->impl.description->shared_pool,
				       in_data_size,
				       0x10, &rq->in_data_allocation);
		if (rc < 0) {
			set_status(status, XRP_STATUS_FAILURE);
			return;
		}
		dsp_cmd->in_data_addr = rq->in_data_allocation->start;
		in_data_ptr = p2v(rq->in_data_allocation->start);
	} else {
		in_data_ptr = &dsp_cmd->in_data;
	}
	dsp_cmd->in_data_size = in_data_size;
	memcpy(in_data_ptr, in_data, in_data_size);
	arch_clean_invalidate_cache_range((addr_t)in_data_ptr, in_data_size);

	if (out_data_size > XRP_DSP_CMD_INLINE_DATA_SIZE) {
		long rc = xrp_allocate(device->impl.description->shared_pool,
				       out_data_size,
				       0x10, &rq->out_data_allocation);
		if (rc < 0) {
			set_status(status, XRP_STATUS_FAILURE);
			return;
		}
		dsp_cmd->out_data_addr = rq->out_data_allocation->start;
		rq->out_data_ptr = p2v(rq->out_data_allocation->start);
	} else {
		rq->out_data_ptr = &dsp_cmd->out_data;
	}
	dsp_cmd->out_data_size = out_data_size;

	if (buffer_group)
		xrp_mutex_lock(&buffer_group->mutex);

	n_buffers = buffer_group ? buffer_group->n_buffers : 0;
	if (n_buffers > XRP_DSP_CMD_INLINE_BUFFER_COUNT) {
		long rc = xrp_allocate(device->impl.description->shared_pool,
				       n_buffers * sizeof(struct xrp_dsp_buffer),
				       0x10, &rq->buffer_allocation);
		if (rc < 0) {
			xrp_mutex_unlock(&buffer_group->mutex);
			set_status(status, XRP_STATUS_FAILURE);
			return;
		}
		dsp_cmd->buffer_addr = rq->buffer_allocation->start;
		rq->buffer_ptr = p2v(rq->buffer_allocation->start);
	} else {
		rq->buffer_ptr = dsp_cmd->buffer_data;
	}
	dsp_cmd->buffer_size = n_buffers * sizeof(struct xrp_dsp_buffer);

	rq->n_buffers = n_buffers;
	rq->user_buffer_allocation = malloc(n_buffers * sizeof(void *));
	for (i = 0; i < n_buffers; ++i) {
		phys_addr_t addr;

		if (buffer_group->buffer[i].buffer->type == XRP_BUFFER_TYPE_DEVICE) {
			/* still through cache on freertos */
			addr = buffer_group->buffer[i].buffer->impl.xrp_allocation->start;
			arch_clean_cache_range((addr_t)p2v(addr),
			                       buffer_group->buffer[i].buffer->size);
		} else {
			if (!xrp_translatable(buffer_group->buffer[i].buffer->ptr)) { // need shadow copy if cannot share
				long rc = xrp_allocate(device->impl.description->shared_pool,
						       buffer_group->buffer[i].buffer->size,
						       0x10, rq->user_buffer_allocation + i);

				if (rc < 0) {
					set_status(status, XRP_STATUS_FAILURE);
					return;
				}
				addr = rq->user_buffer_allocation[i]->start;
				memcpy(p2v(addr), buffer_group->buffer[i].buffer->ptr,
				       buffer_group->buffer[i].buffer->size);
				arch_clean_invalidate_cache_range((addr_t)p2v(addr), 
				                                  buffer_group->buffer[i].buffer->size);
			} else {
				addr = v2p((buffer_group->buffer[i].buffer->ptr));
				arch_clean_cache_range((addr_t)(buffer_group->buffer[i].buffer->ptr),
				                       buffer_group->buffer[i].buffer->size);
			}
		}
		rq->buffer_ptr[i] = (struct xrp_dsp_buffer){
			.flags = buffer_group->buffer[i].access_flags,
			.size  = buffer_group->buffer[i].buffer->size,
			.addr  = addr,
		};
		arch_clean_cache_range((addr_t)(&(rq->buffer_ptr[i])), sizeof(struct xrp_dsp_buffer));
	}

	if (buffer_group)
		xrp_mutex_unlock(&buffer_group->mutex);

	if (evt) {
		struct xrp_event *event = xrp_event_create();

		if (!event) {
			set_status(status, XRP_STATUS_FAILURE);
			return;
		}
		xrp_retain_queue(queue);
		event->queue = queue;
		*evt = event;
		xrp_retain_event(event);
		rq->event = event;
	} else {
		rq->event = NULL;
	}
	dsp_cmd->flags = (queue->use_nsid ? XRP_DSP_CMD_FLAG_REQUEST_NSID : 0);
	if (queue->use_nsid) {
		memcpy(dsp_cmd->nsid, queue->nsid, sizeof(dsp_cmd->nsid));
	}
	set_status(status, XRP_STATUS_SUCCESS);
	xrp_queue_push(&queue->impl.queue, &rq->q);
}
