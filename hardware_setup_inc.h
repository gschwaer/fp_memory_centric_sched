#define CPUECTLR_EL1 S3_1_c15_c2_1
#define TABLE_WALK_DESCR_ACC_PREF (1 << 38)
#define L2_INSTR_FETCH_PREF_DIST_MASK (3 << 35)
#define L2_LDR_STR_DATA_PREF_DIST_MASK (3 << 32)

#define CPUECTLR_EL1 S3_1_c15_c2_1
#define TABLE_WALK_DESCR_ACC_PREF (1 << 38)
#define L2_INSTR_FETCH_PREF_DIST_MASK (3 << 35)
#define L2_LDR_STR_DATA_PREF_DIST_MASK (3 << 32)

#define PMUSERENR_CR_BIT (1 << 2)
#define PMUSERENR_EN_BIT (1 << 0)
#define PMCNTENSET_C_BIT (1 << 31)
#define PMCCFILTR_P_BIT (1 << 31)
#define PMCCFILTR_U_BIT (1 << 30)
#define PMCCFILTR_NSK_BIT (1 << 29)
#define PMCCFILTR_NSU_BIT (1 << 28)
#define PMCCFILTR_NSH_BIT (1 << 27)
#define PMCCFILTR_M_BIT (1 << 26)
#define PMCCFILTR_SH_BIT (1 << 24)
#define PMCR_LC_BIT (1 << 6)
#define PMCR_DP_BIT (1 << 5)
#define PMCR_D_BIT (1 << 3)
#define PMCR_C_BIT (1 << 2)
#define PMCR_E_BIT (1 << 0)
