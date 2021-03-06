#include <stdio.h>
#include <fstream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <string>
#include "Process.h"
#include <deque>

#define unstarted 0
#define running 1
#define ready 2
#define blocked 3
#define terminated 4

using namespace std;
bool verbose=false;
deque<int> run_queue;
vector<Process> all_process;

Process::Process(int i, int A,int B, int C, int M){
	this->index = i;
	this->A = A;
	this->B = B;
	this->C = C;
	this->M = M;
	this->io_t = 0;
	this->current_time = 0;
	this->next_io = 0;
	this->remain_t = C;
	this->state = 0;
	this->wait_t = 0;
	this->finish_t = -1;
	this->q_t = 2;
}

void Process::printProcess(){
	printf(" \n");
    printf("(A,B,C,M) = (%d,%d,%d,%d)\n", A, B, C, M);
    printf("Finishing time: %d\n", finish_t);
    printf("Turnaround time: %d\n", finish_t-A);
    printf("IO time: %d\n", io_t);
    printf("Waiting time: %d\n", wait_t);
    printf("\n");
}

bool cmp(Process x, Process y) {
	return (x.A < y.A || (x.A == y.A && x.index < y.index));
}

bool cmp1(int x, int y){
	return (all_process[x].arrive_t < all_process[y].arrive_t || 
	(all_process[x].arrive_t == all_process[y].arrive_t && x<y));
}

void print_vector(const vector<Process> &v) {
	printf("%d ", v.size());
	for (int i=0; i<v.size(); i++) {
		printf("(%d %d %d %d) ", v[i].A, v[i].B, v[i].C, v[i].M);
	}
	printf("\n");
}

void print_all_process_state(int t,const vector<Process> &all_process){
	printf("Before cycle %d:", t);
	for (int i=0; i<all_process.size(); i++) {
		if (all_process[i].state == unstarted) printf("unstarted %d ", all_process[i].current_time);
		if (all_process[i].state == running) printf("running %d ", all_process[i].q_t);
		if (all_process[i].state == ready) printf("ready 0 ");
		if (all_process[i].state == blocked) printf("blocked %d ", all_process[i].next_io);
		if (all_process[i].state == terminated) printf("terminated %d ", all_process[i].current_time);
	}
	printf("\n");
}

int randomOS(int x, ifstream &random_input) {
	int u;
	random_input >> u;
	return 1 + (u % x);
}

int main(int argc, char** argv){
	ifstream input;
	if (argc==2){
		input.open(argv[1]);
	}
	if (argc==3){
		verbose = true;
		input.open(argv[2]);
	}
//	ifstream input("input-4.txt");
	ifstream random_input("random-numbers.txt");
	int n;
	input >> n;
	for (int i=0;i<n;i++){
		int A,B,C,M;
		input >> A >> B >> C >> M;
		Process temp = Process(i, A, B, C, M);
		all_process.push_back(temp);
	}
	input.close();
	printf("The original input was: ");
	print_vector(all_process);
	sort(all_process.begin(), all_process.end(), cmp);
	printf("The (sorted) input is: ");
	print_vector(all_process);
	printf("\n");
	int finished=0;
	int cur_t;
	if (verbose) printf("This detailed printout gives the state and remaining burst for each process\n\n");
	double tot_cpu=0;
	double tot_io=0;
	bool flag;
	while (finished < n) {
		if (verbose) print_all_process_state(cur_t, all_process);
		flag = false;
		for (int i=0; i<all_process.size(); i++) {
			if (all_process[i].state == blocked) {
				flag = true;
				all_process[i].next_io--;
				all_process[i].io_t++;
			}
			else if (all_process[i].state == ready) all_process[i].wait_t++;
		}
		if (flag) tot_io++;
		flag = false;
		if (!run_queue.empty()) {
			all_process[run_queue[0]].current_time--;
			all_process[run_queue[0]].remain_t--;
			all_process[run_queue[0]].q_t--;
			flag = true;
			if (all_process[run_queue[0]].current_time == 0) {
				if (all_process[run_queue[0]].remain_t == 0){
					all_process[run_queue[0]].state = terminated;
					all_process[run_queue[0]].finish_t = cur_t;
					finished++;
				}
				else{
					all_process[run_queue[0]].state = blocked;
					all_process[run_queue[0]].q_t = min(2, all_process[run_queue[0]].remain_t);
				}
				run_queue.pop_front();
			}
			else if (all_process[run_queue[0]].q_t == 0) {
				all_process[run_queue[0]].state = ready;
				all_process[run_queue[0]].q_t = min(2, all_process[run_queue[0]].current_time);
				all_process[run_queue[0]].arrive_t = cur_t;
				run_queue.push_back(run_queue[0]);
				run_queue.pop_front();				
			}
		}
		if (flag) tot_cpu++;
		for (int i=0; i<all_process.size(); i++) {
			if (all_process[i].A == cur_t || (all_process[i].state == blocked && all_process[i].next_io == 0)){
				all_process[i].state = ready;
				all_process[i].current_time = min(randomOS(all_process[i].B, random_input), all_process[i].remain_t);
				all_process[i].next_io = all_process[i].current_time * all_process[i].M;
				all_process[i].q_t = min(2, all_process[i].current_time); 
				all_process[i].arrive_t = cur_t;
				run_queue.push_back(i);
			}
		}
		sort(run_queue.begin(),run_queue.end(),cmp1);
		if (!run_queue.empty()) all_process[run_queue[0]].state = running;
		cur_t++;
	}
	printf("The scheduling algorithm used was Round Robin\n\n");
	double tot_turnaround=0;
	double tot_waiting=0;
	for (int i=0; i<all_process.size(); i++) {
		printf("Process %d:\n", i);
		all_process[i].printProcess();
		tot_turnaround += all_process[i].finish_t - all_process[i].A;
		tot_waiting += all_process[i].wait_t;
	}
	printf(" \n");
    printf("Summary data:\n");
    printf("Finish time: %d\n", cur_t-1);
    printf("CPU Utilization: %.6f\n", (float) tot_cpu / (cur_t-1));
    printf("IO Utilization: %.6f\n", (float) tot_io / (cur_t-1));
    printf("Throughput: %.6f processes per hundred cycles\n", (float) n / (cur_t-1) * 100);
    printf("Average turnaround time: %.6f\n", (float) tot_turnaround / n);
    printf("Average waiting time: %.6f\n", (float) tot_waiting / n);
	return 0;
}
