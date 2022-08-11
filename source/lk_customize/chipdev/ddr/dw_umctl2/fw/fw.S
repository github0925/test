/********************************************************
 *	    Copyright(c) 2019	Semidrive  Semiconductor    *
 *	    All rights reserved.                            *
 ********************************************************/

	.section ".rodata"
	.align	4

#if LPDDR4X || CFG_DDR_TRAINING_4_ALL
	.global	ddrphy_fw_lpddr4x_dmem
	.global ddrphy_fw_lpddr4x_dmem_end
	.global	ddrphy_fw_lpddr4x_imem
	.global ddrphy_fw_lpddr4x_imem_end
	.global	ddrphy_fw_lpddr4x_2d_dmem
	.global ddrphy_fw_lpddr4x_2d_dmem_end
	.global	ddrphy_fw_lpddr4x_2d_imem
	.global ddrphy_fw_lpddr4x_2d_imem_end
#endif

#if LPDDR4 || CFG_DDR_TRAINING_4_ALL
	.global	ddrphy_fw_lpddr4_dmem
	.global ddrphy_fw_lpddr4_dmem_end
	.global	ddrphy_fw_lpddr4_imem
	.global ddrphy_fw_lpddr4_imem_end
	.global	ddrphy_fw_lpddr4_2d_dmem
	.global ddrphy_fw_lpddr4_2d_dmem_end
	.global	ddrphy_fw_lpddr4_2d_imem
	.global ddrphy_fw_lpddr4_2d_imem_end
#endif

#if DDR4 || CFG_DDR_TRAINING_4_ALL
	.global	ddrphy_fw_ddr4_dmem
	.global ddrphy_fw_ddr4_dmem_end
	.global	ddrphy_fw_ddr4_imem
	.global ddrphy_fw_ddr4_imem_end
	.global	ddrphy_fw_ddr4_2d_dmem
	.global ddrphy_fw_ddr4_2d_dmem_end
	.global	ddrphy_fw_ddr4_2d_imem
	.global ddrphy_fw_ddr4_2d_imem_end
#endif

#if DDR3 || CFG_DDR_TRAINING_4_ALL
	.global	ddrphy_fw_ddr3_dmem
	.global ddrphy_fw_ddr3_dmem_end
	.global	ddrphy_fw_ddr3_imem
	.global ddrphy_fw_ddr3_imem_end
#endif

#if LPDDR4X || CFG_DDR_TRAINING_4_ALL
ddrphy_fw_lpddr4x_dmem:
	.incbin "fw_release/lpddr4x/1d/lpddr4x_pmu_train_dmem.bin"
ddrphy_fw_lpddr4x_dmem_end:
ddrphy_fw_lpddr4x_imem:
	.incbin "fw_release/lpddr4x/1d/lpddr4x_pmu_train_imem.bin"
ddrphy_fw_lpddr4x_imem_end:
ddrphy_fw_lpddr4x_2d_dmem:
	.incbin "fw_release/lpddr4x/2d/lpddr4x_2d_pmu_train_dmem.bin"
ddrphy_fw_lpddr4x_2d_dmem_end:
ddrphy_fw_lpddr4x_2d_imem:
	.incbin "fw_release/lpddr4x/2d/lpddr4x_2d_pmu_train_imem.bin"
ddrphy_fw_lpddr4x_2d_imem_end:
#endif

#if LPDDR4 || CFG_DDR_TRAINING_4_ALL
ddrphy_fw_lpddr4_dmem:
	.incbin "fw_release/lpddr4/1d/lpddr4_pmu_train_dmem.bin"
ddrphy_fw_lpddr4_dmem_end:
ddrphy_fw_lpddr4_imem:
	.incbin "fw_release/lpddr4/1d/lpddr4_pmu_train_imem.bin"
ddrphy_fw_lpddr4_imem_end:
ddrphy_fw_lpddr4_2d_dmem:
	.incbin "fw_release/lpddr4/2d/lpddr4_2d_pmu_train_dmem.bin"
ddrphy_fw_lpddr4_2d_dmem_end:
ddrphy_fw_lpddr4_2d_imem:
	.incbin "fw_release/lpddr4/2d/lpddr4_2d_pmu_train_imem.bin"
ddrphy_fw_lpddr4_2d_imem_end:
#endif

#if DDR4 || CFG_DDR_TRAINING_4_ALL
ddrphy_fw_ddr4_dmem:
	.incbin "fw_release/ddr4/1d/ddr4_pmu_train_dmem.bin"
ddrphy_fw_ddr4_dmem_end:
ddrphy_fw_ddr4_imem:
	.incbin "fw_release/ddr4/1d/ddr4_pmu_train_imem.bin"
ddrphy_fw_ddr4_imem_end:
ddrphy_fw_ddr4_2d_dmem:
	.incbin "fw_release/ddr4/2d/ddr4_2d_pmu_train_dmem.bin"
ddrphy_fw_ddr4_2d_dmem_end:
ddrphy_fw_ddr4_2d_imem:
	.incbin "fw_release/ddr4/2d/ddr4_2d_pmu_train_imem.bin"
ddrphy_fw_ddr4_2d_imem_end:
#endif

#if DDR3 || CFG_DDR_TRAINING_4_ALL
ddrphy_fw_ddr3_dmem:
	.incbin "fw_release/ddr3/1d/ddr3_pmu_train_dmem.bin"
ddrphy_fw_ddr3_dmem_end:
ddrphy_fw_ddr3_imem:
	.incbin "fw_release/ddr3/1d/ddr3_pmu_train_imem.bin"
ddrphy_fw_ddr3_imem_end:
#endif
