extern Frequency MINFREQ;
extern int N, N_TOTAL;
extern double ALPHA, THRESHOLD, COUNT, DELTA;
extern vector<int> CLASS_VEC;

void readClass(char *class_file);
void checkTestable(vector<LegOccurrence>& elements, Frequency frequency);
void checkTestableCl(vector<CloseLegOccurrence>& elements, Frequency frequency);
void checkCondition();
template<typename T> void computePvalue(vector<T>& elements, Frequency frequency);
