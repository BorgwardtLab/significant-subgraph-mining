#include "misc.h"
#include <fstream>
#define PROFILE_MINING

/* This file declares variables which will be used across different C files */

// CODE DEPENDENCIES WITH LCM (typedef statements and such)
// #include"lib_e.c"
// CODE DEPENDENCIES WITH NEW CODE (typedef statements and such)
// #include"transaction_keeping.h"

// VARIABLES DEFINED IN lcm_var.c
// extern int **LCM_Ot, **LCM_Os;
// extern int LCM_th;
// extern ARY LCM_Trsact;
// VARIABLES DEFINED IN transaction_keeping.c
// extern BM_TRANS_LIST current_trans;
// VARIABLES DEFINED IN wy.c
extern int N;
extern int n;
extern long long n_pvalues_computed;
extern long long n_cellcounts_computed;
extern long long effective_total_dataset_frq;
// VARIABLES DEFINED IN permutation.cpp
extern int J;
extern char *labels;
extern char **labels_perm;
extern int *cell_counts;

// occurrence of each subgraph
extern std::vector<Tid> OCC_VEC;
// extern std::ofstream ofs, ofs_p;

extern time_t SEED;
#define TAB "    "

extern int N_SMALL, N_TOTAL;
extern vector<int> CLASS_VEC;
