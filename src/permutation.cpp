#ifndef _permutation_c_
#define _permutation_c_

/* LIBRARY INCLUDES */
#include<stdio.h>
#include<stdlib.h>
#include<time.h>

/* CODE DEPENDENCIES */
#include"permutation.h"
#include"var_declare.h"
#include"misc.h"

/* CONSTANT DEFINES */
#define READ_BUF_SIZ 524288 //Size of the buffer to read chars from file
#define STORE_PERMUTATIONS_FILE 0 //Set the flag to 1 if the generated permutions should be written to a file for reuse/debugging

/* GLOBAL VARIABLES */
// Number of permutations
int J;
//Random generator seed for reproducibility
// time_t SEED;
// Original vector of labels (dimension # non-empty transactions)
char *labels;
// Matrix of size J x (# non-empty transactions)
char **labels_perm;

/* --------------------------------INITIALIZATION AND TERMINATION FUNCTIONS--------------------------------------- */

// Function to initialise core variables
void permutation_init(int n_perm, char *labels_file){
	int i, j; // Iterator variable to be used in loops
	FILE *f_perm = ((FILE*)0);
	char *perm_buffer;
	char *prev_ptr;

	// Obtain number of observations N and number of observations in positive class n
	// from the class labels file
	// get_N_n(labels_file);
	N = N_TOTAL;
	n = N_SMALL;

	// Save core constants as global variables
	J = n_perm;

	// Allocate memory for the vector of labels, giving an error if it fails
	labels = (char *)malloc(N*sizeof(char));
	if(!labels){
		fprintf(stderr,"Error in function permutation_init: couldn't allocate memory for array labels\n");
		exit(1);
	}

	/* Allocate memory for the matrix of permuted labels */
	// First allocate memory for the J row pointers
	// labels_perm = (char **)malloc(root_trans_list.siz1*sizeof(char *));
	labels_perm = (char **)malloc(N * sizeof(char *));
	if(!labels_perm){
		fprintf(stderr,"Error in function permutation_init: couldn't allocate memory for array labels_perm\n");
		exit(1);
	}
	// Now allocate memory for a contiguous block of J*(# non-empty transactions) chars
	labels_perm[0] = (char *)malloc(((long long)J) * N *sizeof(char));
	if(!labels_perm[0]){
		fprintf(stderr,"Error in function permutation_init: couldn't allocate memory for array labels_perm[0]\n");
		exit(1);
	}
	// And make each row pointer point to the appropriate position (labels_perm[0] is
	// already correctly set)
	for(i=1;i<N;i++) labels_perm[i] = labels_perm[0] + ((long long)i)*J;

	// Read file containing class labels and store them in array labels
	// read_labels_file(labels_file);
	for (int i = 0; i < N_TOTAL; i++) {
	  labels[i] = CLASS_VEC[i];
	}
	

	// Initialize random number generator
	// SEED = time(NULL);
	srand(SEED);

	/* Compute the random permutations of the class labels */
	// Create file for storing the permutations (only if STORE_PERMUTATIONS_FILE==1)
	if(STORE_PERMUTATIONS_FILE){
		f_perm = fopen("permutations.csv","w");
		// Check if file was correctly created, otherwise raise an error
		if(!f_perm){
			fprintf(stderr,"Error in function permutation_init: file %s couldn't be created\n","permutations.csv");
			exit(1);
		}
	}

	// Do the J permutations themselves, storing them in labels_perm and
	// saving them to the file f_perm if necessary
	perm_buffer = (char *)malloc(N*sizeof(char));
	if(!perm_buffer){
		fprintf(stderr,"Error in function permutation_init: couldn't allocate memory for array perm_buffer\n");
		exit(1);
	}
	for(j=0;j<J;j++) {
		randperm(perm_buffer, labels, N, f_perm);
		// Dump contents of buffer into destination, skipping values corresponding to empty observations/transactions
		for(i=0;i<N;i++) labels_perm[i][j] = perm_buffer[i];
	}
	free(perm_buffer);
	// The array containing the indices of all non-empty transactions is no longer needed
	// free(non_empty_trans_idx);

	// Close the file f_perm (if necessary)
	if(f_perm) fclose(f_perm);
}

// Function that free all allocated memory
void permutation_end(){
	int i;// Iterator variable to be used in loops
	// Free allocated memory for class labels
	free(labels);
	// Free the main memory block for the permuted labels matrix
	free(labels_perm[0]);
	// Free the array of row pointers
	free(labels_perm);

	// Report the seed for reproducibility
	// printf("\nRANDOM SEED USED FOR KEY GENERATION\n");
	// printf("\t Seed = %u\n",(unsigned)seed);
	// ofs << "seed_for_permutation\t" << (unsigned)SEED << endl;
}

/* Do a first scan of the file containing the class labels to compute the total number of observations, N,
 * and the total number of observations in the positive class, n
 * */
void get_N_n(char *labels_file){
	FILE *f_labels;//Stream with file containing class labels
	int n_read;//Number of chars read
	int i;// Iterator variable to be used in loops
	char char_to_int[256];//Array for converting chars to int fast
	char *read_buf, *read_buf_aux, *read_buf_end;//Buffer for reading from file and extra pointers for loops

	// Initialise both counters to 0 (the variables are defined as global variables in wy.c)
	N = 0; n = 0;

	//Try to open file, giving an error message if it fails
	if(!(f_labels = fopen(labels_file,"r"))){
		fprintf(stderr, "Error in function get_N_n when opening file %s\n",labels_file);
		exit(1);
	}

	//Try to allocate memory for the buffer, giving an error message if it fails
	read_buf = (char *)malloc(READ_BUF_SIZ*sizeof(char));
	if(!read_buf){
		fprintf(stderr,"Error in function read_labels_file: couldn't allocate memory for array read_buf\n");
		exit(1);
	}

	//Initialize the char to int converter
	for(i=0;i<256;i++) char_to_int[i] = 127;
	// We only care about the chars '0' and '1'. Everything else is mapped into the same "bucket"
	char_to_int['0'] = 0; char_to_int['1'] = 1;

	// Read the entire file
	while(1){
		// Try to read READ_BUF_SIZ chars from the file containing the class labels
		n_read = fread(read_buf,sizeof(char),READ_BUF_SIZ,f_labels);
		// If the number of chars read, n_read_ is smaller than READ_BUF_SIZ, either the file ended
		// or there was an error. Check if it was the latter
		if((n_read < READ_BUF_SIZ) && !feof(f_labels)){
			fprintf(stderr,"Error in function read_labels_file while reading the file %s\n",labels_file);
			exit(1);
		}
		// Process the n_read chars read from the file
		for(read_buf_aux=read_buf,read_buf_end=read_buf+n_read;read_buf_aux<read_buf_end;read_buf_aux++){
			//If the character is anything other than '0' or '1' go to process the next char
			if(char_to_int[*read_buf_aux] == 127) continue;
			N++;
			if(char_to_int[*read_buf_aux]) n++;
		}
		// Check if the file ended,. If yes, then exit the while loop
		if(feof(f_labels)) break;
	}

	//Close the file
	fclose(f_labels);

	//Free allocated memory
	free(read_buf);
}

void read_labels_file(char *labels_file){
	FILE *f_labels;//Stream with file containing class labels
	int n_read;//Number of chars read
	int i;// Iterator variable to be used in loops
	char char_to_int[256];//Array for converting chars to int fast
	char *read_buf, *read_buf_aux, *read_buf_end;//Buffer for reading from file and extra pointers for loops
	char *labels_aux = labels;//Auxiliary pointer to array labels for increments

	//Try to open file, giving an error message if it fails
	if(!(f_labels = fopen(labels_file,"r"))){
		fprintf(stderr, "Error in function read_labels_file when opening file %s\n",labels_file);
		exit(1);
	}

	//Try to allocate memory for the buffer, giving an error message if it fails
	read_buf = (char *)malloc(READ_BUF_SIZ*sizeof(char));
	if(!read_buf){
		fprintf(stderr,"Error in function read_labels_file: couldn't allocate memory for array read_buf\n");
		exit(1);
	}

	//Initialize the char to int converter
	for(i=0;i<256;i++) char_to_int[i] = 127;
	// We only care about the chars '0' and '1'. Everything else is mapped into the same "bucket"
	char_to_int['0'] = 0; char_to_int['1'] = 1;

	// Read the entire file
	while(1){
		// Try to read READ_BUF_SIZ chars from the file containing the class labels
		n_read = fread(read_buf,sizeof(char),READ_BUF_SIZ,f_labels);
		// If the number of chars read, n_read_ is smaller than READ_BUF_SIZ, either the file ended
		// or there was an error. Check if it was the latter
		if((n_read < READ_BUF_SIZ) && !feof(f_labels)){
			fprintf(stderr,"Error in function read_labels_file while reading the file %s\n",labels_file);
			exit(1);
		}
		// Process the n_read chars read from the file
		for(read_buf_aux=read_buf,read_buf_end=read_buf+n_read;read_buf_aux<read_buf_end;read_buf_aux++){
			//If the character is anything other than '0' or '1' go to process the next char
			if(char_to_int[*read_buf_aux] == 127) continue;
			*labels_aux++ = char_to_int[*read_buf_aux];
		}
		// Check if the file ended,. If yes, then exit the while loop
		if(feof(f_labels)) break;
	}

	//Close the file
	fclose(f_labels);

	//Free allocated memory
	free(read_buf);
}

/* -----------------FUNCTIONS TO SAMPLE RANDOM INTEGERS AND GENERATE RANDOM PERMUTATIONS--------------------------- */

/* Sample a random integer uniformly distributed the range [0,x)
 * Default random number generator samples integers uniformly in the range [0,RAND_MAX)
 * To sample in the range [0,x) with x < RAND_MAX, one can think of sampling an integer
 * rnd from [0,RAND_MAX) and then returning rnd % x. However, this may generate a non-uniform
 * distribution unless RAND_MAX is a multiple of x. To fix that, we combine that basic idea
 * with rejection sampling, rejecting any value rnd greater or equal than RAND_MAX - RAND_MAX *x.
 * This is the same as ensuring that the "effective" RAND_MAX is an exact multiple of x, so that
 * returning rnd % x leads to a uniform sampling scheme in the range [0,x)
 * The seed of the random number generator should have been initialised externally
 * */
static int rand_int(int x){
	int rnd;
	int limit = RAND_MAX - RAND_MAX % x;

	do{
		rnd = rand();
	}while(rnd >= limit);
	return rnd % x;
}

void randperm(char *buffer, char *src, int l, FILE *f_perm){
	int i,j; // Variables for looping and swapping
	char tmp; // Temp int for swapping
	//Array to store the permutation and temp int for swapping (only needed if f_perm is not NULL)
	int *perm_array, tmp_int;

	// First of all, copy the original array in the buffer
	for(i=0;i<l;i++) buffer[i] = src[i];

	// If the generated permutation is to be stored in a file, initialise memory for perm_array and int_to_str
	if(f_perm){
		perm_array = (int *)malloc(l*sizeof(int));
		if(!perm_array){
			fprintf(stderr,"Error in function randperm: couldn't allocate memory for array perm_array\n");
			exit(1);
		}
		// Initialise indices of the permutation to the identity permutation
		for(i=0;i<l;i++) perm_array[i] = i;
	}

	// Fisher-Yates algorithm
	for(i = l-1; i > 0; i--){
		// Sample a random integer in [0,i]
		j = rand_int(i + 1);
		// Swap dest[j] and dest[i]
		tmp = buffer[j];
		buffer[j] = buffer[i];
		buffer[i] = tmp;
		// If the generated permutation has to be written to the stream f_perm, keep
		// track of the indices as well
		if(f_perm){
			tmp_int = perm_array[j];
			perm_array[j] = perm_array[i];
			perm_array[i] = tmp_int;
		}
	}
	// If the generated permutation is to be stored in a file, write perm_array to the stream
	if(f_perm){
		// Write the first l-1 indices with comma as a delimiter
		for(i=0;i<(l-1);i++) fprintf(f_perm,"%d,",perm_array[i]);
		// For the last index, change the comma by a newline char
		fprintf(f_perm,"%d\n",perm_array[i]);
		// Free allocated memory
		free(perm_array);
	}
}

#endif
