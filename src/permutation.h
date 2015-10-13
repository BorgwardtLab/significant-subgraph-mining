/* FUNCTION DECLARATIONS */
void get_N_n(char *);
void read_labels_file(char *);
void randperm(char *, char *, int, FILE *);
void permutation_init(int n_perm, char *labels_file);
void permutation_end();
static int rand_int(int x);
void randperm(char *buffer, char *src, int l, FILE *f_perm);
