#ifndef _wy_c_
#define _wy_c_

/* LIBRARY INCLUDES */
#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<iostream>
#include<fstream>

/* CODE DEPENDENCIES */
#include"var_declare.h"
#include"permutation.h"
// #include"transaction_keeping.c"
// #include"lcm_var.c"
#include"wy.h"
#include"misc.h"

/* CONSTANT DEFINES */


/* GLOBAL VARIABLES */
// Number of observations, N; and midpoint of interval [0,N], floor(N/2)
int N, N_over_2;
// Number of observations in positive class
int n;
// Target FWER
double alpha;
// Current FWER
double FWER;

// Minimum P-value for each permutation
double *min_pval;
// Region thresholds: Sigma_k = [sl1,sl2] U [N-sl2,N-sl1]
int sl1, sl2;
// Current P-value threshold
double delta;
// Flag variable to keep track of the last change done to region Sigma_k
// If flag==1, the last change was sl1++ (shrink on extremes of the W)
// If flag==0, the last change was sl2-- (shrink on center of the W)
int flag;

// Array with all values of log(n!) in [0,N] pre-computed
double *loggamma;
// Logarithm of 1/binom(N,n). This terms appears for every evaluation of the hypergeometric
// PDF, so it makes sense to precompute
double log_inv_binom_N_n;
// Array with all values of minimum attainable P-value in [0,N] pre-computed
double *psi;
// Array for storing values of the CDF of the hypergeometric distribution for fast p-valaue computation
double *hypergeom_cdf;
double *hypergeom_cdf_inverse;

// Cell-count counter
int *a_cnt;

// Profiling variables
long long n_pvalues_computed;
long long n_cellcounts_computed;
long long effective_total_dataset_frq;

/* -------------------------------- INITIALISATION AND TERMINATION FUNCTIONS ----------------------------------------- */

/* Initialise the Westfall-Young permutation code
 * Input arguments are self-explanatory
 * */
void wy_init(double target_fwer){
	int j; //Loop variable

	// Store core constants
	N_over_2 = (N % 2) ? (N-1)/2 : N/2;//floor(N/2)
	alpha = target_fwer;
	// And initialise some others
	sl1 = 1; sl2 = N_over_2;
	flag = 1;
	FWER = 0;
	delta = ((double) n)/N; //$\psi(1)=\frac{n}{N}$

	// Initialise cache for log(x!) and psi(x)
	loggamma_init();
	psi_init();

	// Allocate memory for minimum p-values, raising error if it fails
	min_pval = (double *)malloc(J*sizeof(double));
	if(!min_pval){
		fprintf(stderr,"Error in function wy_init: couldn't allocate memory for array min_pval\n");
		exit(1);
	}
	// Initialise all p-values to 1
	for(j=0; j<J; j++) min_pval[j] = 1;

	// Allocate memory for the CDF of the hypergeometric distribution
	// (worst case memory requirement n+1), raising an error if it fails
	hypergeom_cdf = (double *)malloc((n+1)*sizeof(double));
	if(!hypergeom_cdf){
		fprintf(stderr,"Error in function wy_init: couldn't allocate memory for array hypergeom_cdf\n");
		exit(1);
	}
	hypergeom_cdf_inverse = (double *)malloc((n+1)*sizeof(double));
	if(!hypergeom_cdf_inverse){
		fprintf(stderr,"Error in function wy_init: couldn't allocate memory for array hypergeom_cdf_inverse\n");
		exit(1);
	}

	// Allocate memory for cell counts, raising an error if it fails
	a_cnt = (int *)malloc(J*sizeof(int));
	if(!a_cnt){
		fprintf(stderr,"Error in function wy_init: couldn't allocate memory for array a_cnt\n");
		exit(1);
	}
	for(j=0; j<J; j++) a_cnt[j] = 0;

	n_pvalues_computed = 0; n_cellcounts_computed = 0; effective_total_dataset_frq = 0; //Init profiling variables
}

/* Precompute values of log(x!) storing them in the array loggamma */
void loggamma_init(){
	int x;
	// Allocate memory for log-gamma cache, raising error if it fails
	loggamma = (double *)malloc((N+1)*sizeof(double));
	if(!loggamma){
		fprintf(stderr,"Error in function loggamma_init: couldn't allocate memory for array loggamma\n");
		exit(1);
	}
	// Initialise cache with appropriate values
	for(x=0;x<=N;x++) loggamma[x] = lgamma(x+1);//Gamma(x) = (x-1)!
	// Initialise log_inv_binom_N_n
	log_inv_binom_N_n = loggamma[n] + loggamma[N-n] - loggamma[N];
}

/* Precompute minimum attainable P-values $\psi(x)$ for all x in [0,N] and store them in array psi */
void psi_init(){
	double xi1;
	int x, x_init;
	// Allocate memory for psi, raising error if it fails
	psi = (double *)malloc((N+1)*sizeof(double));
	if(!psi){
		fprintf(stderr,"Error in function psi_and_xi1_init: couldn't allocate memory for array psi\n");
		exit(1);
	}

	/* Initialise caches with appropriate values */

	// First compute the left side of "the W", i.e. the range [0,n]
	psi[0] = 1;
	//In this range, the recursion $\psi(x)$=$\psi(x-1)$*[(n-(x-1))/(N-(x-1))] can be seen to be correct
	for(x=1; x<=n; x++) psi[x] = (((double)(n-(x-1)))/(N-(x-1)))*psi[x-1];

	// Now, start computing xi1 in the range [N-N_over_2,N] using another recursion, this time
	// starting in N
	// Note that we don't need to store all values, since this will be used only to initialise
	// psi[N_over_2]
	x_init = N-N_over_2;
	xi1 = 1;
	//In this range, the recursion $\xi_{1}(x)$=$\xi_{1}(x+1)$*[((x-1)-n)/(x+1)] can be seen to be correct
	for(x=(N-1); x>=x_init; x--) xi1 = (((double)((x+1)-n))/(x+1))*xi1;

	// Now, use the value of $\xi_{1}(N-N_over_2)$=xi1[0] to get $\psi(N_over_2)$=psi[N_over_2] using the
	// same recursion if N is odd, or simply copy the value of xi1[0] since $\xi_{1}(N-N_over_2)=\psi(N_over_2)$
	// if N is even
	if (N % 2) psi[N_over_2] = (((double)(x_init-n))/x_init)*xi1;
	else psi[N_over_2] = xi1;

	// Now, using psi[N_over_2] compute the right side of "the W", i.e. the range [n+1,N_over_2]
	// using the same recursion as for $\xi_{1}$
	for(x=(N_over_2-1); x > n; x--) psi[x] = (((double)((x+1)-n))/(x+1))*psi[x+1];

	// Finally, since $\psi(x)$ is symmetric around N_over_2, complete the right half by symmetry
	for(x=x_init; x<=N; x++) psi[x] = psi[N-x];
}

/* Free all allocated memory and give some output for debugging purposes */
void wy_end(){
	int j, idx_max;
	double delta_corrected;
	// Sort p-values
	qsort(min_pval,J,sizeof(double),doublecomp);
	// Tentative index to corrected significance threshold
	idx_max = (int)floor(alpha*J)-1; delta_corrected = min_pval[idx_max];
	// Check and correct (if necessary) boundary cases
	if(delta_corrected==min_pval[idx_max+1]){
		while(min_pval[--idx_max]==delta_corrected);
		delta_corrected = min_pval[idx_max];
	}
	DELTA = delta_corrected;
	
	// Print results
	/*
	cerr << endl << "Results:" << endl
	     << TAB << "Corrected significance threshold:         " << delta_corrected << endl
	     << TAB << "FWER at corrected significance threshold: " << floor(idx_max+1)/J << endl
	     << TAB << "Final minfreq:                            " << minfreq << endl
	     << TAB << "Final P-value lower bound:                " << delta << endl
	     << TAB << "FWER at final P-value lower bound:        " << FWER << endl;
	*/

	/*
	for(j=0;j<(J-1);j++) ofs_p << min_pval[j] << " ";
	ofs_p << endl;
	*/

	// Free allocated memory
	free(loggamma);
	free(psi);
	free(min_pval);
	free(hypergeom_cdf);
	free(hypergeom_cdf_inverse);
	free(a_cnt);
}

/* -------------------------------FUNCTIONS TO COMPUTE FISHER EXACT TEST P-VALUES----------------------------------- */

/* This function precomputes all Fisher exact test P-values for a contingency table with margins x,n,N that is,
 * all p-values p(a,x,n,N) for a in the range [max(0,n+x-N),min(x,n)]. The results will be stored in the array
 * hypergeom_cdf such that p(a,x,n,N)=hypergeom_cdf[a]. Note that values hypergeom_cdf[a] for a outside
 * [max(0,n+x-N),min(x,n)] are undefined and could contain garbage of previous hypotheses.
 * */
void precompute_pvals(int x){
	double pre_comp_xterms;
	int a, a_min, a_max;
	// Compute the contribution of all terms depending on x but not on a
	pre_comp_xterms = loggamma[x] + loggamma[N-x];
	a_min = ((n+x-N) > 0) ? (n+x-N) : 0;//max(0,n+x-N)
	a_max = (x > n) ? n : x;//min(x,n)
	hypergeom_cdf[a_min] = exp(pre_comp_xterms + log_inv_binom_N_n - (loggamma[a_min] + loggamma[n-a_min] + loggamma[x-a_min] + loggamma[(N-n)-(x-a_min)]));
	hypergeom_cdf_inverse[a_max] = exp(pre_comp_xterms + log_inv_binom_N_n - (loggamma[a_max] + loggamma[n-a_max] + loggamma[x-a_max] + loggamma[(N-n)-(x-a_max)]));
	hypergeom_cdf[a_max] = 1; hypergeom_cdf_inverse[a_min] = 1;
	for(a=(a_min+1);a<a_max;a++) hypergeom_cdf[a] = hypergeom_cdf[a-1] + exp(pre_comp_xterms + log_inv_binom_N_n - (loggamma[a] + loggamma[n-a] + loggamma[x-a] + loggamma[(N-n)-(x-a)]));
	for(a=(a_max-1);a>a_min;a--) hypergeom_cdf_inverse[a] = hypergeom_cdf_inverse[a+1] + exp(pre_comp_xterms + log_inv_binom_N_n - (loggamma[a] + loggamma[n-a] + loggamma[x-a] + loggamma[(N-n)-(x-a)]));
	for(a=a_max;a>a_min;a--) hypergeom_cdf[a] = (hypergeom_cdf[a] < hypergeom_cdf_inverse[a]) ? hypergeom_cdf[a] : hypergeom_cdf_inverse[a];
}

/* --------------------------------CORE FAST WESTFALL-YOUNG PERMUTATION FUNCTIONS------------------------------------ */

/* Decrease the minimum p-value threshold one level
 * Main operations that need to be performed are:
 * 1) Figure out whether we have to shrink "the W" on the left side or the right side, that is, if the current region
 *    is Sigma_{k} = [sl1,sl2] U [N-sl2,N-sl1], we need to figure out if Sigma_{k+1} is of the form
 *    Sigma_{k+1} = [sl1+1,sl2] U [N-sl2,N-sl1-1] (shrink left side) or Sigma_{k+1} = [sl1,sl2-1] U [N-sl2+1,N-sl1-1]
 *    (shrink right side). This is done with help of a binary flag that remembers which of the two types of region
 *    change happened the last time the threshold was decreased.
 * 2) Update variables sl1, sl2 and delta accordingly
 * 3) If sl1 has been modified, then the support of LCM has to be modified
 * 4) Since the temptative corrected significance threshold delta has changed, the FWER needs to be recomputed
 * */
void decrease_threshold(){
	int j; //Loop iterator
	int false_positives; //Number of false positives (a false positive occurs if min_pval[j] <= delta)
	// Flag==1 means the last call to decrease_threshold() shrunk "the W" on the left side
	if(flag){
		sl1++; // Shrink Sigma_k on extremes of the W
		// Check what the new case will be
		if (psi[sl1] >= psi[sl2]) delta = psi[sl1];
		else{ delta = psi[sl2]; flag = 0; }
		//Update LCM minimum support
		// LCM_th = sl1;
		minfreq = sl1;
		// printf("\n\n\nTHRESHOLD CHANGE!!! NEW THRESHOLD=%d\n\n\n",LCM_th);
		// cerr << minfreq << " ";
		cerr << "<<< Update minfreq to " << minfreq << " >>>" << endl;
	}else{ // Flag==0 means the last call to decrease_threshold() shrunk "the W" on the right side
		sl2--; // Shrink Sigma_k on center of the W
		// Check what the new case will be
		if (psi[sl1] >= psi[sl2]){ delta = psi[sl1]; flag = 1; }
		else delta = psi[sl2];
		//No need to update LCM minimum support in this case, since sl1 remains the same
	}
	// Recompute FWER from scratch
	false_positives = 0;
	for(j=0; j<J; j++) false_positives += (min_pval[j]<=delta) ? 1 : 0;
	FWER = ((double)false_positives)/J;
}


/* -------------------FUNCTIONS TO PROCESS A NEWLY FOUND TESTABLE HYPOTHESIS-------------------------------------- */

/* This code contains 3 difference functions to process newly found hypotheses. All of them are virtually identical
 * and the only thing which differs is the way the function receives the list of observations (transactions) for
 * which the hypothesis has X=1.
 * LCM has a complex structure, with frequent itemsets being found at 4 different locations in the source code
 * and under 3 different circumstances. Therefore it was needed to introduce differentiation in the way the transaction
 * list is fed to the "solution processing functions" in order to keep the "transaction keeping" overhead minimal.
 *
 * To reuse this code for problems other than frequent itemset mining, the only thing that needs to be modified
 * is the line which computes the cell counts, for example, the following line in bm_process_solution:
 * 		for(i=0; i<current_trans.siz; i++) a += labels_perm[j][current_trans.list[i]];
 * 	There, current_trans.siz is the number of transactions for which the hypothesis has X=1, i.e. the margin x
 * 	of the 2x2 contingency table (note in this case it is redundant with the input argument x of the function)
 * 	Similarly, current_trans.list[i] with i ranging from 0 to (x-1) has the list of indices of the observations
 * 	for which X=1.
 * 	Simply changing those two parameters accordingly allows code reuse.
 * */

/* Process a solution involving the bitmap represented itemsets */
// x = frequency (i.e. number of occurrences) of newly found solution
// void bm_process_solution(int x, int item, int *mask){
void bm_process_solution(int x, int item){
	int i,j;//Loop iterators
	double pval; //Variable to hold p-values
	char *labels_perm_aux;
	/* First, process the new hypothesis */

	// Minimum attainable P-value for the hypothesis
	double psi_x = psi[x];
	// Check if the newly found solution is in the current testable region Sigma_k
	if(psi_x > delta) return;

	// Precompute CDF and p-values of hypergeometric distribution with frequency x
	precompute_pvals(x);
	n_pvalues_computed++; //Update profiling variable

	// Compute cell-counts for all J-permutations
	// for(i=0; i<current_trans.siz; i++){
	for(i=0; i<x; i++){
		labels_perm_aux = labels_perm[OCC_VEC[i]];//Avoid recomputing labels_perm[current_trans.list[i]] all the time
		for(j=0; j<J; j++) a_cnt[j] += labels_perm_aux[j];
	}
	n_cellcounts_computed += J; //Update profiling variable
	effective_total_dataset_frq += x; // Update profiling variable

	// If not, compute permuted P-values for each permutation
	for(j=0; j<J; j++){
		// Fetch the precomputed p-value
		pval = hypergeom_cdf[a_cnt[j]];
		// Sanity-check
		if(pval < 0) printf("Negative P-value detected in bm_process_solution!: j=%d, x=%d, a=%d, pval=%e.\n",j,x,a_cnt[j],pval);
		a_cnt[j] = 0;
		// Check if the newly computed p-value is smaller than current minimum p-value for the
		// permutation
		if(pval < min_pval[j]){
			// Check if the decrease in the current minimum p-value for the j-th permutation
			// causes an increase in the FWER
			if( (pval<=delta) && (min_pval[j]>delta)) FWER += ((double)1)/J;
			min_pval[j] = pval;
		}
	}

	/* Finally, check if the FWER constraint is still satisfied, if not decrease threshold */
	while(FWER > alpha) {
		//printf("Threshold change BM\n");
		decrease_threshold();
		// Correct possible corruption of LCM data structures due to unexpected change in minimum support
		/*
		for(i=0; i<item; i++){
			//printf("Item %d, Frq %d, Current_th %d\n",i,LCM_Ofrq[i],LCM_th);
			if(LCM_Ofrq[i]==(LCM_th-1)){
				//printf("Bucket of item %d corrupted after TH change! Parent %d. Current Th%d.\n",i,item,LCM_th);
				LCM_BM_occurrence_delete(i);
				*mask &= ~BITMASK_1[i];
				//printf("Problem fixed!\n");
			}
		}
		*/
	}
}


/* AUXILIARY FUNCTIONS */
// Comparison function used by quicksort implementation in C
int doublecomp(const void* elem1, const void* elem2){
    if(*(const double*)elem1 < *(const double*)elem2)
        return -1;
    return *(const double*)elem1 > *(const double*)elem2;
}

#endif
