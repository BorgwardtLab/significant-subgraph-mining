extern Frequency MINFREQ;
extern int N, N_TOTAL;
extern double ALPHA, THRESHOLD, COUNT;
extern vector<int> CLASS_VEC;

void readClass(char *class_file);
void checkTestable(vector<LegOccurrence>& elements, Frequency frequency);
void checkTestableCl(vector<CloseLegOccurrence>& elements, Frequency frequency);
template<typename T> void checkCondition(vector<T>& elements, Frequency frequency);
