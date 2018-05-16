
//This program will simulate the execution of a series of input processes and utilize scheduling accordingly to the times requested
#include <iostream>
#include <algorithm>
#include <vector>
#include <queue>
#include <fstream>
#include <string>
#include <iomanip>

using namespace std;

int totalProcess = 0, currentProcess = 0, CoreTime = 0, SSDTimes = 0, SSDCount= 0, SSDAccess = 0;
int NCORES;

class Process //Class created for processes according to time, queue will use this
{
	int processCount, seriesCount, time;
	string procType;
public:
	Process(int tempprocessCount, int tempseriesCount, string tempprocType, int temptime)
	{
		procType = tempprocType;
		time = temptime;
		processCount = tempprocessCount;
		seriesCount = tempseriesCount;
	}
	string getprocType() const { 
		return procType; }
	int gettime() const { 
		return time; }
	int getprocessCount() const { 
		return processCount; }
	int getseriesCount() const { 
		return seriesCount; }
};

class msCheck //checks and compare time of processes
{
public:
	bool operator()(const Process& process1, const Process& process2) {
		return process1.gettime() > process2.gettime();
	}
};

void request(const Process& process);
void Tracker();
void Table();
void InputComplete();
queue<Process> SSDQ; //queue for SSD
queue<Process> InputQ; //qeueu for Input
queue<Process> ReadyQ; //queue for Ready
priority_queue<Process, vector<Process>, msCheck> ProcQ; //queue for process
vector<string> processes; //store processes
vector<pair<string, string> > R;
vector< vector<pair<string, string> > > PTable; //Table to hold processes and time status

string tempname, temptime;
int inputlines = 0;

void Table() { //Parse in input file and Creates a data table of the process(s)
	while (cin >> tempname >> temptime) {
		inputlines++;
		if (inputlines > 2048) {
			cout << "Too many input lines" << endl;
		}
		if (totalProcess > 256) {
			cout << "Too many processes" << endl;
		}
		for (unsigned int i = 0; i < tempname.size(); i++) {
			tempname.at(i) = static_cast<char>(toupper(tempname.at(i)));
		}
		if (tempname == "NCORES") {
			NCORES = stoi(temptime);
		}
		else if (tempname == "NEW") {
			totalProcess++;
			processes.push_back("OPEN");
			if (R.empty()) {
				R.push_back(make_pair(tempname, temptime));
			}
			else {
				PTable.push_back(R);
				R.clear();
				R.push_back(make_pair(tempname, temptime));
			}
		}
		else if (tempname == "CORE") {
			R.push_back(make_pair(tempname, temptime));
		}
		else if (tempname == "SSD") {
			R.push_back(make_pair(tempname, temptime));
		}
		else if (tempname == "INPUT") {
			R.push_back(make_pair(tempname, temptime));
		}
		else {
			cout << "input error" << endl;
		}
	}
	PTable.push_back(R);
	R.clear();
}

bool SSDopen = true;
bool INPUTopen = true;
void Tracker() { //Tracks and determines status and check for the next process available in queue, after finishing, requests for next event to be processed
	Process proc = ProcQ.top();
	ProcQ.pop();
	int timeComplete;
	if (proc.getprocType() == "CoreCompletion") {
		if (ReadyQ.empty()) {
			NCORES++;
		}
		else {
			Process Queue = ReadyQ.front();
			ReadyQ.pop();
			timeComplete = Queue.gettime() + proc.gettime();
			ProcQ.push(Process(Queue.getprocessCount(), Queue.getseriesCount(), Queue.getprocType(), timeComplete));
			processes[Queue.getprocessCount()] = "RUNNING";
		}
		request(proc);
	}
	else if (proc.getprocType() == "SSDCompletion") {
		SSDTimes += proc.gettime();
		if (SSDQ.empty()) {
			SSDopen = true;
		}
		else {
			Process Queue = SSDQ.front();
			SSDQ.pop();
			timeComplete = Queue.gettime() + proc.gettime();
			ProcQ.push(Process(Queue.getprocessCount(), Queue.getseriesCount(), Queue.getprocType(), timeComplete));
			processes[Queue.getprocessCount()] = "BLOCKED";
		}
		request(proc);
	}
	else if (proc.getprocType() == "InputCompletion") {
		if (InputQ.empty()) {
			INPUTopen = true;
		}
		else {
			Process Queue = InputQ.front();
			InputQ.pop();
			timeComplete = Queue.gettime() + proc.gettime();
			ProcQ.push(Process(Queue.getprocessCount(), Queue.getseriesCount(), Queue.getprocType(), timeComplete));
			processes[Queue.getprocessCount()] = "BLOCKED";
		}
		request(proc);
	}
	else {
		cout << "error" << endl;
	}
}

void InputComplete() { //Incorporates a new process if possible and then keeps track of process counts until finished, prints out the process # and status of it
	int seriesCount = 0;
	if (currentProcess < totalProcess) {
		int ms = stoi(PTable[currentProcess][seriesCount].second);
		if (ReadyQ.empty() || ms < ProcQ.top().gettime()) {
			if (NCORES > 0) {
				NCORES--;
				seriesCount++;
				int finishTime = ms + stoi(PTable[currentProcess][seriesCount].second);
				CoreTime += stoi(PTable[currentProcess][seriesCount].second);
				cout << endl << "Process " << currentProcess << " starts at time " << ms << "ms" << endl;
				for (int i = 0; i < processes.size(); i++)
				{
					string status = processes[i];
					if (status == "OPEN" || i == currentProcess) {
					}
					else {
						cout << "Process " << i << " is " << status << endl;
					}
				}
				processes[currentProcess] = "RUNNING";
				ProcQ.push(Process(currentProcess, seriesCount, "CoreCompletion", finishTime));
				currentProcess++;
				InputComplete();
			}
			else {
				cout << endl << "Process " << currentProcess << " starts at time " << ms << "ms" << endl;
				for (int i = 0; i < processes.size(); i++)
				{
					string status = processes[i];
					if (status == "OPEN" || i == currentProcess) {
					}
					else {
						cout << "Process " << i << " is " << status << endl;
					}
				}
				seriesCount++;
				Process proc = Process(currentProcess, seriesCount, "CoreCompletion", stoi(PTable[currentProcess][seriesCount].second));
				CoreTime += stoi(PTable[currentProcess][seriesCount].second);
				ReadyQ.push(proc);
				processes[currentProcess] = "READY";
				currentProcess++;

				InputComplete();
			}
		}
		else {
			Tracker();
		}
	}
	else {
		Tracker();
	}
}

void  request(const Process& proc) { //Requests for next process event, and checks for completion of input and whether the program is finished. Prints out Summary of process statistics after completion
	if (PTable[proc.getprocessCount()].size() == (proc.getseriesCount() + 1)) {
		cout << endl << "Process " << proc.getprocessCount() << " terminates at time " << proc.gettime() << "ms" << endl;
		processes[proc.getprocessCount()] = "TERMINATED";
		for (int i = 0; i < processes.size(); i++)
		{
			string status = processes[i];
			if (status == "OPEN") {
			}
			else {
				cout << "Process " << i << " is " << status << endl;
			}
		}
		processes[proc.getprocessCount()] = "OPEN";
		if (ProcQ.empty() && ReadyQ.empty() && SSDQ.empty() && InputQ.empty()) {
			cout << endl << "SUMMARY: " << endl;

			cout << "Number of processes that completed: " << totalProcess << endl;

			cout << "Total number of SSD accesses: " << SSDAccess << endl;

			float CoreUtil = (float)CoreTime / (float)proc.gettime();

			float SSDAvg = (float)SSDTimes / (float)SSDAccess;

			float SSDUtil = (float)SSDCount / (float)proc.gettime();

			cout << fixed;
			cout << setprecision(2);

			cout << "Average SSD access time: " << SSDAvg << " ms" << endl;
			cout << "Total elapsed time: " << proc.gettime() << " ms" << endl;
			cout << "Core utilization: " << CoreUtil * 100 << " percent" << endl;
			cout << "SSD utilization: " << SSDUtil * 100 << " percent" << endl;
			return;
		}
	}
	else {
		string requestType = PTable[proc.getprocessCount()][proc.getseriesCount() + 1].first;
		int timeRequest = stoi(PTable[proc.getprocessCount()][proc.getseriesCount() + 1].second);
		int timeComplete = proc.gettime() + timeRequest;
		if (requestType == "INPUT") {
			processes[proc.getprocessCount()] = "BLOCKED";
			if (INPUTopen) {
				INPUTopen = false;
				ProcQ.push(Process(proc.getprocessCount(), proc.getseriesCount() + 1, "InputCompletion", timeComplete));
			}
			else {
				InputQ.push(Process(proc.getprocessCount(), proc.getseriesCount() + 1, "InputCompletion", timeRequest));
			}
		}
		else if (requestType == "CORE") {
			if (NCORES > 0) {
				NCORES--;
				CoreTime += stoi(PTable[proc.getprocessCount()][proc.getseriesCount() + 1].second);
				ProcQ.push(Process(proc.getprocessCount(), proc.getseriesCount() + 1, "CoreCompletion", timeComplete));
				processes[proc.getprocessCount()] = "RUNNING";
			}
			else {
				ReadyQ.push(Process(proc.getprocessCount(), proc.getseriesCount() + 1, "CoreCompletion", timeRequest));
				CoreTime += timeRequest;
				processes[proc.getprocessCount()] = "READY";
			}
		}
		else if (requestType == "SSD") {
			SSDAccess++;
			SSDTimes -= proc.gettime();
			SSDCount += timeRequest;
			processes[proc.getprocessCount()] = "BLOCKED";
			if (SSDopen) {
				SSDopen = false;
				ProcQ.push(Process(proc.getprocessCount(), proc.getseriesCount() + 1, "SSDCompletion", timeComplete));
			}
			else {
				SSDQ.push(Process(proc.getprocessCount(), proc.getseriesCount() + 1, "SSDCompletion", timeRequest));
			}
		}
		else {
			cout << "error" << endl;
		}
	}
	InputComplete();
}
int main() {

	Table();

	InputComplete();

	return 0;
}