/*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
*/

#ifndef SX_DMA_H
#define SX_DMA_H

#include <debug.h>
#include <ce.h>

/**
 * @brief Configure fetch and push operations in direct mode on internal DMA
 * @param vce_id      vce index
 * @param src block_t to the source data to transfer
 * @param dst block_t to the destination location
 * @param length the length in bytes to transfer (from src to dest)
 */
void cryptodma_config(uint32_t vce_id,
                      block_t dst,
                      block_t src,
                      uint32_t length);

/**
 * @brief Check cryptodma status
 * Trigger a hardfault if any error occured
 * @param vce_id      vce index
 */
uint32_t cryptodma_check_status(uint32_t vce_id) ;

/**
 * @brief Clear DMA error bits
 * @param vce_id      vce index
 */
void cryptodma_err_clear(uint32_t vce_id);

/** @brief Start internal DMA transfer
 * @param vce_id      vce index
 */
void cryptodma_start(uint32_t vce_id);

/** @brief Wait until internal DMA is done
 * @param vce_id      vce index */
void cryptodma_wait(uint32_t vce_id);

/**
 * @brief Check cryptodma error flag
 * @param vce_id      vce index
 * @return CRYPTOLIB_DMA_ERR if fifo's are not empty, CRYPTOLIB_SUCCESS otherwise
 */
uint32_t cryptodma_check_bus_error(uint32_t vce_id);

/**
 * @brief copy block with DMA
 * @param vce_id      vce index
 * @param dst         destination block
 * @param src         source block
 * @param length      length of operation
 * @return CRYPTOLIB_DMA_ERR if fifo's are not empty, CRYPTOLIB_SUCCESS otherwise
 */
uint32_t memcpy_blk(uint32_t vce_id,
                    block_t dst,
                    block_t src,
                    uint32_t length);

uint32_t memcpy_blk_cache(uint32_t vce_id,
                          block_t dst,
                          block_t src,
                          uint32_t length,
                          bool cache_op_src,
                          bool cache_op_dst);
/**
 * Copy an ECC point (X and Y coordinates) to CryptoRAM at two consecutive
 * locations. The last \p size bytes from each of the two CryptoRAM locations
 * will be written. Should be used when big-endianness is considered.
 * @param vce_id      vce index
 * @param src point on the curve: X coordinate followed by Y coordinate,
 *            each having  \p size bytes
 * @param size size of the operand/coordinate in bytes
 * @param offset position of the first coordinate in cryptoRAM where the
 *               X coordinate will be copied; the Y coordinate will be
 *               written at location offset + 1
 *        (from ::BA414EP_MEMLOC_0 to ::BA414EP_MEMLOC_14)
 */
uint32_t point2CryptoRAM_rev(uint32_t vce_id,
                             block_t src,
                             uint32_t size,
                             uint32_t offset);

/**
 * Copy ECC point (X and Y coordinates) from two consecutive locations in the
 * CryptoRAM. The last \p size bytes from each of the two memory locations will
 * be transferred. Should be used when big-endianness is considered.
 * @param vce_id      vce index
 * @param dst location where the X coordinate followed by the Y coordinate will
 *            be stored
 * @param size size of the operand/coordinate in bytes
 * @param offset position of the first coordinate in cryptoRAM where the X
 *               coordinate is stored; the Y coordinate will be copied from
 *               the location offset + 1
 *        (from ::BA414EP_MEMLOC_0 to ::BA414EP_MEMLOC_14)
 */
uint32_t CryptoRAM2point_rev(uint32_t vce_id,
                             block_t dst,
                             uint32_t size,
                             uint32_t offset);

/**
 * Copy an ECC point (X and Y coordinates) to two consecutive CryptoRAM
 * locations. The first \p size bytes of each location will be written. Should
 * be used when little-endianness is considered.
 * @param vce_id      vce index
 * @param src point on the curve: X coordinate followed by Y coordinate,
 *            each having  \p size bytes
 * @param size size of the operand/coordinate in bytes
 * @param offset position of the first coordinate in cryptoRAM where the
 *               X coordinate will be copied; the Y coordinate will be
 *               written at location offset + 1
 *        (from ::BA414EP_MEMLOC_0 to ::BA414EP_MEMLOC_14)
 */
uint32_t point2CryptoRAM(uint32_t vce_id,
                         block_t src,
                         uint32_t size,
                         uint32_t offset);

/**
 * Copy ECC point (X and Y coordinates) from two consecutive locations in the
 * CryptoRAM. The first \p size bytes of the two memory locations will be
 * transferred. Should be used when little-endianness is considered.
 * @param vce_id      vce index
 * @param dst location where the X coordinate followed by the Y coordinate will
 *            be stored
 * @param size size of the operand/coordinate in bytes
 * @param offset position of the first coordinate in cryptoRAM where the X
 *               coordinate is stored; the Y coordinate will be copied from
 *               the location offset + 1
 *        (from ::BA414EP_MEMLOC_0 to ::BA414EP_MEMLOC_14)
 */
uint32_t CryptoRAM2point(uint32_t vce_id,
                         block_t dst,
                         uint32_t size,
                         uint32_t offset);

/**
 * @brief Transfer data from memory to the end of a CryptoRAM location.
 *        Should be used when big-endianness is considered.
 *        Data from src are padded from src.len to size with zeros.
 * @param vce_id      vce index
 * @param src is a data block of the source that has to be copied (byte-addressing)
 * @param size is the size of the data to copy expressed in bytes
 * @param offset is the offset of the memory location that has to be copied
 *        (from ::BA414EP_MEMLOC_0 to ::BA414EP_MEMLOC_15)
 */
uint32_t mem2CryptoRAM_rev(uint32_t vce_id,
                           block_t src,
                           uint32_t size,
                           uint32_t offset,
                           bool cache_op);

/**
 * @brief Transfer data from the end of a CryptoRAM location to memory.
 * Should be used when big-endianness is considered.
 * @param vce_id      vce index
 * @param dst is a block_t where the source has to be copied to (byte-addressing)
 * @param size is the size of the data to copy expressed in bytes
 * @param offset is the offset of the memory location that has to be copied
 *        (from ::BA414EP_MEMLOC_0 to ::BA414EP_MEMLOC_15)
 */
uint32_t CryptoRAM2mem_rev(uint32_t vce_id,
                           block_t dst,
                           uint32_t size,
                           uint32_t offset,
                           bool cache_op);

/**
 * @brief Transfer data from memory to CryptoRAM starting at the beginning of
 * the CryptoRAM location. Should be used when little endianness is
 * considered.
 * @param vce_id      vce index
 * @param src is a data block of the source that has to be copied (byte-addressing)
 * @param size is a the size of the data to copy expressed in bytes
 * @param offset is the offset of the memory location that has to be copied
 *        (from ::BA414EP_MEMLOC_0 to ::BA414EP_MEMLOC_15)
 */
uint32_t mem2CryptoRAM(uint32_t vce_id,
                       block_t src,
                       uint32_t size,
                       uint32_t offset,
                       bool cache_op);

/**
 * @brief Function transferring data from CryptoRAM to memory starting from the
 * beginning of the CryptoRAM location. Should be used when little endianness is
 * considered.
 * @param vce_id      vce index
 * @param dst is a block_t where the source has to be copied to (byte-addressing)
 * @param size is the size of the data to copy expressed in bytes
 * @param offset is the offset of the memory location that has to be copied
 *        (from ::BA414EP_MEMLOC_0 to ::BA414EP_MEMLOC_15)
 */
uint32_t CryptoRAM2mem(uint32_t vce_id,
                       block_t dst,
                       uint32_t size,
                       uint32_t offset,
                       bool cache_op);
/**
 * @brief Function compare data with reversal order
 *
 * @param src is a data sequence as comparing reference
 * @param dst is a data sequence as comparing objects
 * @param len is the size of the data to compare expressed in bytes
 */
int memcmp_rev(uint8_t* src,
               const uint8_t* dst,
               uint32_t len);

#endif
