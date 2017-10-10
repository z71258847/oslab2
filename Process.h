class Process{
public:
	Process(int i, int A,int B, int C, int M);
	int A, B, C, M;
	int io_t;
	int wait_t;
	int remain_t;
	int finish_t;
	int state;
	int index;
	int current_time;
	int next_io;
	int q_t;
	int arrive_t;
	
	void printProcess();
};
