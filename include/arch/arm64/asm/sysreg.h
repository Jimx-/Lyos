#ifndef _ARCH_SYSREG_H_
#define _ARCH_SYSREG_H_

#define SCTLR_ELx_ATA (BIT(43))

#define SCTLR_ELx_ENIA_SHIFT 31

#define SCTLR_ELx_ITFSB (BIT(37))
#define SCTLR_ELx_ENIA  (BIT(SCTLR_ELx_ENIA_SHIFT))
#define SCTLR_ELx_ENIB  (BIT(30))
#define SCTLR_ELx_ENDA  (BIT(27))
#define SCTLR_ELx_EE    (BIT(25))
#define SCTLR_ELx_IESB  (BIT(21))
#define SCTLR_ELx_WXN   (BIT(19))
#define SCTLR_ELx_ENDB  (BIT(13))
#define SCTLR_ELx_I     (BIT(12))
#define SCTLR_ELx_SA    (BIT(3))
#define SCTLR_ELx_C     (BIT(2))
#define SCTLR_ELx_A     (BIT(1))
#define SCTLR_ELx_M     (BIT(0))

#define SCTLR_EL1_EPAN (BIT(57))
#define SCTLR_EL1_ATA0 (BIT(42))

#define SCTLR_EL1_BT1     (BIT(36))
#define SCTLR_EL1_BT0     (BIT(35))
#define SCTLR_EL1_UCI     (BIT(26))
#define SCTLR_EL1_E0E     (BIT(24))
#define SCTLR_EL1_SPAN    (BIT(23))
#define SCTLR_EL1_NTWE    (BIT(18))
#define SCTLR_EL1_NTWI    (BIT(16))
#define SCTLR_EL1_UCT     (BIT(15))
#define SCTLR_EL1_DZE     (BIT(14))
#define SCTLR_EL1_UMA     (BIT(9))
#define SCTLR_EL1_SED     (BIT(8))
#define SCTLR_EL1_ITD     (BIT(7))
#define SCTLR_EL1_CP15BEN (BIT(5))
#define SCTLR_EL1_SA0     (BIT(4))

#define SCTLR_EL1_RES1 \
    ((BIT(11)) | (BIT(20)) | (BIT(22)) | (BIT(28)) | (BIT(29)))

#define INIT_SCTLR_EL1_MMU_ON                                             \
    (SCTLR_ELx_M | SCTLR_ELx_C | SCTLR_ELx_SA | SCTLR_EL1_SA0 |           \
     SCTLR_EL1_SED | SCTLR_ELx_I | SCTLR_EL1_DZE | SCTLR_EL1_UCT |        \
     SCTLR_EL1_NTWE | SCTLR_ELx_IESB | SCTLR_EL1_SPAN | SCTLR_ELx_ITFSB | \
     SCTLR_ELx_ATA | SCTLR_EL1_ATA0 | SCTLR_EL1_UCI | SCTLR_EL1_EPAN |    \
     SCTLR_EL1_RES1)

#define MAIR_ATTR_DEVICE_nGnRnE UL(0x00)
#define MAIR_ATTR_DEVICE_nGnRE  UL(0x04)
#define MAIR_ATTR_NORMAL_NC     UL(0x44)
#define MAIR_ATTR_NORMAL_TAGGED UL(0xf0)
#define MAIR_ATTR_NORMAL        UL(0xff)
#define MAIR_ATTR_MASK          UL(0xff)

#define MAIR_ATTRIDX(attr, idx) ((attr) << ((idx)*8))

/* id_aa64mmfr0 */
#define ID_AA64MMFR0_ECV_SHIFT       60
#define ID_AA64MMFR0_FGT_SHIFT       56
#define ID_AA64MMFR0_EXS_SHIFT       44
#define ID_AA64MMFR0_TGRAN4_2_SHIFT  40
#define ID_AA64MMFR0_TGRAN64_2_SHIFT 36
#define ID_AA64MMFR0_TGRAN16_2_SHIFT 32
#define ID_AA64MMFR0_TGRAN4_SHIFT    28
#define ID_AA64MMFR0_TGRAN64_SHIFT   24
#define ID_AA64MMFR0_TGRAN16_SHIFT   20
#define ID_AA64MMFR0_BIGENDEL0_SHIFT 16
#define ID_AA64MMFR0_SNSMEM_SHIFT    12
#define ID_AA64MMFR0_BIGENDEL_SHIFT  8
#define ID_AA64MMFR0_ASID_SHIFT      4
#define ID_AA64MMFR0_PARANGE_SHIFT   0

#define ID_AA64MMFR0_PARANGE_32  0x0
#define ID_AA64MMFR0_PARANGE_36  0x1
#define ID_AA64MMFR0_PARANGE_40  0x2
#define ID_AA64MMFR0_PARANGE_42  0x3
#define ID_AA64MMFR0_PARANGE_44  0x4
#define ID_AA64MMFR0_PARANGE_48  0x5
#define ID_AA64MMFR0_PARANGE_52  0x6
#define ID_AA64MMFR0_PARANGE_MAX ID_AA64MMFR0_PARANGE_48

#endif
