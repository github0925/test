/*
 * Copyright (c) 2018 Cadence Design Systems Inc.
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
#include <string.h>
#include <lib/elf.h>

#include <xrp_types.h>
#include "xrp_alloc.h"
#include "xrp_host.h"
#include "xrp_kernel_dsp_interface.h"

#include <err.h>
#include <debug.h>
#include "errno.h"
#include "res.h"
#include "image_cfg.h"

void *p2v(phys_addr_t addr)
{
	return (void *)ap2p((paddr_t)addr);
}

phys_addr_t v2p(const void *p)
{
	return (phys_addr_t)p2ap((paddr_t)p);
}

void xrp_initialize_shmem(void)
{
}

/* check whether p can share to vdsp or not, need a shadow copy if not */
int xrp_translatable(const void *p)
{
	return (((paddr_t)p) > ap2p(VDSP_MEMBASE) && ((paddr_t)p) < ap2p(VDSP_MEMBASE + VDSP_MEMSIZE)) ||
	       (((paddr_t)p) > ap2p(VDSP_SHARE_MEMBASE) && ((paddr_t)p) < ap2p(VDSP_SHARE_MEMBASE + VDSP_SHARE_MEMSIZE)) ||
	       (((paddr_t)p) > ap2p(SERVICES_MEMBASE) && ((paddr_t)p) < ap2p(SERVICES_MEMBASE + SERVICES_MEMSIZE));
}

static inline bool xrp_section_bad(size_t size, const struct Elf32_Shdr *shdr)
{
	return shdr->sh_offset > size ||
	       shdr->sh_size > size - shdr->sh_offset;
}

static int xrp_firmware_find_symbol(uint32_t data, size_t size, const char *name,
				    void **paddr, size_t *psize)
{
	const struct Elf32_Ehdr *ehdr      = (struct Elf32_Ehdr *)data;
	const void              *shdr_data = (void*)data + ehdr->e_shoff;
	const struct Elf32_Shdr *sh_symtab = NULL;
	const struct Elf32_Shdr *sh_strtab = NULL;
	const void *sym_data;
	const void *str_data;
	const struct Elf32_Sym *esym;
	void *addr = NULL;
	unsigned i;

	if (ehdr->e_shoff == 0) {
		dprintf(CRITICAL, "%s: no section header in the firmware image\n", __func__);
		return -ENOENT;
	}
	if (ehdr->e_shoff > size ||
	    ehdr->e_shnum * ehdr->e_shentsize > size - ehdr->e_shoff) {
		dprintf(CRITICAL, "%s: bad firmware SHDR information\n", __func__);
		return -EINVAL;
	}

	/* find symbols and string sections */
	for (i = 0; i < ehdr->e_shnum; ++i) {
		const struct Elf32_Shdr *shdr = shdr_data + i * ehdr->e_shentsize;

		switch (shdr->sh_type) {
		case SHT_SYMTAB:
			sh_symtab = shdr;
			break;
		case SHT_STRTAB:
			sh_strtab = shdr;
			break;
		default:
			break;
		}
	}

	if (!sh_symtab || !sh_strtab) {
		dprintf(CRITICAL, "%s: no symtab or strtab in the firmware image\n", __func__);
		return -ENOENT;
	}

	if (xrp_section_bad(size, sh_symtab)) {
		dprintf(CRITICAL, "%s: bad firmware SYMTAB section information\n", __func__);
		return -EINVAL;
	}

	if (xrp_section_bad(size, sh_strtab)) {
		dprintf(CRITICAL, "%s: bad firmware STRTAB section information\n", __func__);
		return -EINVAL;
	}

	/* iterate through all symbols, searching for the name */

	sym_data = (void*)data + sh_symtab->sh_offset;
	str_data = (void*)data + sh_strtab->sh_offset;

	for (i = 0; i < sh_symtab->sh_size; i += sh_symtab->sh_entsize) {
		esym = sym_data + i;

		if (!(ELF32_ST_TYPE(esym->st_info) == STT_OBJECT &&
		      esym->st_name < sh_strtab->sh_size &&
		      strncmp(str_data + esym->st_name, name,
			      sh_strtab->sh_size - esym->st_name) == 0))
			continue;

		if (esym->st_shndx > 0 && esym->st_shndx < ehdr->e_shnum) {
			const struct Elf32_Shdr *shdr = shdr_data +
				esym->st_shndx * ehdr->e_shentsize;
			Elf32_Off in_section_off = esym->st_value - shdr->sh_addr;

			if (xrp_section_bad(size, shdr)) {
				dprintf(CRITICAL, "%s: bad firmware section #%d information\n",
					__func__, esym->st_shndx);
				return -EINVAL;
			}

			if (esym->st_value < shdr->sh_addr ||
			    in_section_off > shdr->sh_size ||
			    esym->st_size > shdr->sh_size - in_section_off) {
				dprintf(CRITICAL, "%s: bad symbol information\n",
					__func__);
				return -EINVAL;
			}
			addr = (void *)data + shdr->sh_offset +	in_section_off;

			dprintf(INFO, "%s: found symbol, st_shndx = %d, "
				"sh_offset = 0x%08x, sh_addr = 0x%08x, "
				"st_value = 0x%08x, address = %p\n",
				__func__, esym->st_shndx, shdr->sh_offset,
				shdr->sh_addr, esym->st_value, addr);
		} else {
			dprintf(CRITICAL, "%s: unsupported section index in found symbol: 0x%x\n",
				__func__, esym->st_shndx);
			return -EINVAL;
		}
		break;
	}

	if (!addr)
		return -ENOENT;

	*paddr = addr;
	*psize = esym->st_size;

	return 0;
}

static int xrp_firmware_fixup_symbol(uint32_t data, size_t size, const char *name,
				     phys_addr_t v)
{
	u32 v32 = XRP_DSP_COMM_BASE_MAGIC;
	void *addr;
	size_t sz;
	int rc;

	rc = xrp_firmware_find_symbol(data, size, name, &addr, &sz);
	if (rc < 0) {
		dprintf(CRITICAL, "%s: symbol \"%s\" is not found\n",
			__func__, name);
		return rc;
	}

	if (sz != sizeof(u32)) {
		dprintf(CRITICAL, "%s: symbol \"%s\" has wrong size: %zu\n",
			__func__, name, sz);
		return -EINVAL;
	}

	/* update data associated with symbol */

	if (memcmp(addr, &v32, sz) != 0) {
		dprintf(INFO, "%s: value pointed to by symbol is incorrect: %p, fix anyway\n",
			__func__, (int)sz, addr);
	}

	v32 = v;
	memcpy(addr, &v32, sz);

	return 0;
}

int xrp_load_firmware(uint32_t data, size_t size, phys_addr_t comm_phys)
{
	struct Elf32_Ehdr *ehdr = (struct Elf32_Ehdr *)data;
	int i;

	if (memcmp(ehdr->e_ident, ELF_MAGIC, 4)) {
		dprintf(CRITICAL, "bad firmware ELF magic\n");
		return -EINVAL;
	}

	if (ehdr->e_type != ET_EXEC) {
		dprintf(CRITICAL, "bad firmware ELF type\n");
		return -EINVAL;
	}

	if (ehdr->e_machine != 94 /*EM_XTENSA*/) {
		dprintf(CRITICAL, "bad firmware ELF machine\n");
		return -EINVAL;
	}

	if (ehdr->e_phoff >= size ||
	    ehdr->e_phoff +
	    ehdr->e_phentsize * ehdr->e_phnum > size) {
		dprintf(CRITICAL, "bad firmware ELF PHDR information\n");
		return -EINVAL;
	}

	xrp_firmware_fixup_symbol(data, size, "xrp_dsp_comm_base", comm_phys);

	return 0;
}

void xrp_exit(void)
{
}
