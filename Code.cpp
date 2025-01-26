#include <iostream> 
#include <fstream> 
#include <string> 
#include <sstream> 
#include <vector> 
#include <algorithm>
#include <iomanip>
using namespace std;

class Job{ 
private: 
    int jobId; 
    int arrivalDay; 
    int timeHour; 
    int memoryRequired;  // in GB 
    int CPURequired;     // in cores
    int executionTime;  // in hours

public:
    //Constructor
    Job(int id,int day,int hour,int memory,int CPU,int execution) : jobId(id),arrivalDay(day),timeHour(hour),memoryRequired(memory),CPURequired(CPU),executionTime(execution) {}

    //Display job details
    void displayJobDetails() const {
        cout << "JobId: " << jobId
             << ", ArrivalDay: " << arrivalDay
             << ", TimeHour: " << timeHour
             << ", MemReq: " << memoryRequired
             << ", CPUReg: " << CPURequired
             << ", ExeTime: " << executionTime
             << endl;
    }

    //Getters for sorting
    int getArrivalDay() const { return arrivalDay; }
    int getTimeHour() const { return timeHour; }
    int getExecutionTime() const { return executionTime; }
    int getJobSize() const { return executionTime*CPURequired*memoryRequired; }
    int getMemoryRequired() const { return memoryRequired; }
    int getCPURequired() const { return CPURequired; }
};

//Worker Node Class
class WorkerNode{
private:
    int nodeId;
    int availableMemory;
    int availableCPU;
    vector<Job> jobs;

public:
    WorkerNode(int id):nodeId(id),availableMemory(64),availableCPU(24){}

    //Add a job to the worker node
    bool addJob(const Job job){
        if(job.getMemoryRequired()<=availableMemory && job.getCPURequired()<=availableCPU){
            jobs.push_back(job);
            availableMemory -= job.getMemoryRequired();
            availableCPU -= job.getCPURequired();
            return true;
        }
        return false;
    }

    //Process all assigned jobs
    void processJobs(const string& policy) const {
        if(!jobs.empty()){
            cout<<"Worker"<<nodeId<<" ("<<policy<<") : "<<endl;
            for(const auto &job:jobs){
            job.displayJobDetails();
            }
        }
    }

    //Reset resources for next round of job allocation
    void resetResources(){
        availableMemory = 64;
        availableCPU = 24;
        jobs.clear();
    }

    //Getters for sorting
    int getAvailableMemory() const { return availableMemory; }
    int getAvailableCPU() const { return availableCPU; }
    int getUsedMemory() const { return 64-availableMemory; }
    int getUsedCPU() const { return 24-availableCPU; }

    const vector<Job>& getJobs() const { return jobs; }
};

//Comparison functions for Sorting
bool compareByArrival(const Job &j1,const Job &j2){
    return(j1.getArrivalDay()<j2.getArrivalDay()) || 
          (j1.getArrivalDay()==j2.getArrivalDay() && j1.getTimeHour()<j2.getTimeHour());
}

bool compareByExecutionTime(const Job& j1,const Job& j2){
    return j1.getExecutionTime()<j2.getExecutionTime();
}

bool compareByJobSize(const Job& j1,const Job& j2){
    return j1.getJobSize()<j2.getJobSize();
}

int main(){
    const int numWorkerNodes = 128;                   // Number of worker nodes
    vector<Job> MasterQueue;                          // Master List to store all jobs
    vector<WorkerNode> workers;

    //Create worker nodes
    for(int i=0;i<numWorkerNodes;++i){
        workers.emplace_back(i+1);
    }

    ifstream file("JobArrival.txt");
    if(!file.is_open()){
        cout<<"File not found!"<<endl;
        return 1;
    }

    string line;
    while(getline(file,line)){
        if(line.find("JobId")!=string::npos){
            int jobId = 0,arrivalDay = 0,timeHour = 0,memReq = 0,cpuReg = 0,exeTime = 0;

            string key;
            stringstream ss(line);

            while(ss>>key){
                if(key == "JobId:") ss >> jobId;
                else if(key == "Arrival"){
                    ss>>key;
                    ss>>arrivalDay;
                }
                else if(key == "Time"){
                    ss>>key;
                    ss>>timeHour;
                }
                else if(key == "MemReq:") ss >> memReq;
                else if(key == "CPUReg:") ss >> cpuReg;
                else if(key == "ExeTime:") ss >> exeTime;
            }
            
            //Create job object and add to job list
            MasterQueue.emplace_back(jobId,arrivalDay,timeHour,memReq,cpuReg,exeTime);
        }
    }
    file.close();

    //Function to process jobs based on given queue
    auto processJobs=[&](vector<Job>& sortedJobs,const string& policy){
        vector<Job> unallocatedJobs;

        for(auto& worker:workers){
            worker.resetResources();
        }

        //Assigning jobs to worker nodes
        for(const auto& job:sortedJobs){
            bool allocated = false;

            //First Fit Policy
            for(auto& worker:workers){
                if(worker.addJob(job)){
                    allocated = true;
                    break;
                }
            }

            //Best Fit Policy
            if(!allocated){
                auto bestFitWorker = workers.end();
                for(auto it=workers.begin();it!=workers.end();++it){
                    if(it->getAvailableMemory()>=job.getMemoryRequired() && it->getAvailableCPU()>=job.getCPURequired()){
                        if(bestFitWorker==workers.end() || 
                        (it->getAvailableMemory()<bestFitWorker->getAvailableMemory()) || 
                        (it->getAvailableMemory()==bestFitWorker->getAvailableMemory() && it->getAvailableCPU()<bestFitWorker->getAvailableCPU())){
                            bestFitWorker=it;
                        }
                    }
                }
                if(bestFitWorker!=workers.end()){
                    bestFitWorker->addJob(job);
                    allocated=true;
                }
            }

            //Worst Fit Policy
            if(!allocated){
                auto worstFitWorker=workers.end();
                for(auto it=workers.begin();it!=workers.end();++it){
                    if(it->getAvailableMemory()>=job.getMemoryRequired() && it->getAvailableCPU()>=job.getCPURequired()){
                        if(worstFitWorker==workers.end() || 
                        (it->getAvailableMemory()>worstFitWorker->getAvailableMemory()) || 
                        (it->getAvailableMemory()==worstFitWorker->getAvailableMemory() && it->getAvailableCPU()>worstFitWorker->getAvailableCPU())){
                            worstFitWorker=it;
                        }
                    }
                }
                if(worstFitWorker!=workers.end()){
                    worstFitWorker->addJob(job);
                    allocated=true;
                }
            }
            if(!allocated){
                unallocatedJobs.push_back(job);
            }
        }
        for(const auto& worker:workers){
            worker.processJobs(policy);
        }

        //Sort unallocated jobs using Combined Policies
        if(!unallocatedJobs.empty()){
            sort(unallocatedJobs.begin(),unallocatedJobs.end(),[](const Job& j1,const Job& j2){
                if(j1.getArrivalDay()!=j2.getArrivalDay()){
                    return j1.getArrivalDay()<j2.getArrivalDay();            //FCFS
                }
                if(j1.getJobSize()!=j2.getJobSize())
                    return j1.getJobSize()<j2.getJobSize();                  //Smallest Job First
                return j1.getExecutionTime()<j2.getExecutionTime();          //Shortest Job First
            });

        //Reinsert sorted unallocated jobs into the MasterQueue
        MasterQueue.insert(MasterQueue.end(),unallocatedJobs.begin(),unallocatedJobs.end());    
        }
    };

    ofstream csvfile("output.csv");
    if(!csvfile.is_open()){
        cout<<"Failed to open CSV File!"<<endl;
        return 1;
    }

    csvfile<<"Day,Memory Utilization (%),CPU Utilization (%)"<<endl;

    int dayCount=1;
    while(!MasterQueue.empty()){
        cout<<"Day "<<dayCount<<endl;

        for(auto &worker:workers){
            worker.resetResources();
        }

        vector<Job> unallocatedJobs;
        int totalMemoryUsed=0;
        int totalCPUUsed=0;

        for(const auto &job:MasterQueue){
            bool allocated = false;

            for(auto &worker:workers){
                if(worker.addJob(job)){
                    allocated = true;
                    break;
                }
            }
            if(!allocated){
                unallocatedJobs.push_back(job);
            }
        }
        for(const auto &worker:workers){
            worker.processJobs("Day "+to_string(dayCount));
            totalMemoryUsed+=worker.getUsedMemory();
            totalCPUUsed+=worker.getUsedCPU();
        }

        MasterQueue = unallocatedJobs;
        double avgMemoryUtilization=static_cast<double>(totalMemoryUsed)/(numWorkerNodes*64)*100.0;
        double avgCPUUtilization=static_cast<double>(totalCPUUsed)/(numWorkerNodes*24)*100.0;

        csvfile<<dayCount<<","<<fixed<<setprecision(2)<<avgMemoryUtilization<<","<<avgCPUUtilization<<endl;

        cout<<fixed<<setprecision(2);
        cout<<"Average Memory Utilization on Day "<<dayCount<<": "<<avgMemoryUtilization<<"%"<<endl;
        cout<<"Average CPU Utilization on Day "<<dayCount<<": "<<avgCPUUtilization<<"%"<<endl;
        ++dayCount;

        cout<<"Reamining Jobs: "<<MasterQueue.size()<<endl;
    }
    csvfile.close();

    //Process Jobs based on Combined Policies
    vector<Job> combinedJobs = MasterQueue;
    processJobs(combinedJobs,"Combined Policies");

    cout<<"All Jobs Processed!"<<endl;

    return 0;
}
