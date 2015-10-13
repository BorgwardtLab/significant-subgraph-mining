extern Frequency MINFREQ;
extern int N_SMALL, N_TOTAL;
extern double ALPHA, THRESHOLD, COUNT, DELTA;
extern vector<int> CLASS_VEC;
extern vector<Tid> OCC_VEC;

void readClass(char *class_file);
void outputSubgraph(Frequency frequency);
void checkTestable(vector<LegOccurrence>& elements, Frequency frequency);
void checkTestableCl(vector<CloseLegOccurrence>& elements, Frequency frequency);
void checkCondition();
template<typename T> void checkConditionWY(vector<T>& elements, Frequency frequency);
template<typename T> void computePvalue(vector<T>& elements, Frequency frequency);
