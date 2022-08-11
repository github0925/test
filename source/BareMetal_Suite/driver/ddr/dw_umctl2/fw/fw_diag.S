/********************************************************
 *	    Copyright(c) 2019	Semidrive  Semiconductor    *
 *	    All rights reserved.                            *
 ********************************************************/

	.section "ddr_diag_sec"
	.align	4

	.global	ddrphy_fw_lpddr4x_diag_dmem
	.global ddrphy_fw_lpddr4x_diag_dmem_end
	.global	ddrphy_fw_lpddr4x_diag_imem
	.global ddrphy_fw_lpddr4x_diag_imem_end

	.global	ddrphy_fw_lpddr4_diag_dmem
	.global ddrphy_fw_lpddr4_diag_dmem_end
	.global	ddrphy_fw_lpddr4_diag_imem
	.global ddrphy_fw_lpddr4_diag_imem_end

	.global	ddrphy_fw_ddr4_diag_dmem
	.global ddrphy_fw_ddr4_diag_dmem_end
	.global	ddrphy_fw_ddr4_diag_imem
	.global ddrphy_fw_ddr4_diag_imem_end

ddrphy_fw_lpddr4x_diag_dmem:
	.incbin "fw_release/lpddr4x/diag/lpddr4x_diags_dmem.bin"
ddrphy_fw_lpddr4x_diag_dmem_end:
ddrphy_fw_lpddr4x_diag_imem:
	.incbin "fw_release/lpddr4x/diag/lpddr4x_diags_imem.bin"
ddrphy_fw_lpddr4x_diag_imem_end:

ddrphy_fw_lpddr4_diag_dmem:
	.incbin "fw_release/lpddr4/diag/lpddr4_diags_dmem.bin"
ddrphy_fw_lpddr4_diag_dmem_end:
ddrphy_fw_lpddr4_diag_imem:
	.incbin "fw_release/lpddr4/diag/lpddr4_diags_imem.bin"
ddrphy_fw_lpddr4_diag_imem_end:

ddrphy_fw_ddr4_diag_dmem:
	.incbin "fw_release/ddr4/diag/ddr4_diags_dmem.bin"
ddrphy_fw_ddr4_diag_dmem_end:
ddrphy_fw_ddr4_diag_imem:
	.incbin "fw_release/ddr4/diag/ddr4_diags_imem.bin"
ddrphy_fw_ddr4_diag_imem_end:
