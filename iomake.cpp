#include <iostream>
#include <fstream>
#include <deque>
#include <string>
#include <sstream>
#include <vector>
#include <stdlib.h>
#include <set>
#include <math.h>
#include <unistd.h> 
#include <stdio.h>
#include <cstring>
#include <algorithm>
#include <climits>

using namespace std;

void printInput();
class Scheduler;
class FIFO;
class SSTF;
class LOOK;
class CLOOK;
class FLOOK;

typedef struct {
    int id;
    int arrival_time;
    int track;
    int end_time;
    int start_time;
    int turnaround;
    int wait_time;
} request;

string INPUT_FILE;
deque<request> request_l;
char* SCHEDULER_TYPE;
Scheduler* SCHEDULER;
request* CURRENT_REQ;
int req_cnt = 0;
int req_total = 0;
int total_move = 0;
int total_time = 0;
int head = 0;
int dir = 1;
bool WHETHER_VERBOSE=false;
bool WHETHER_QUEUE=false;
bool WHETHER_FLOOK=false;

class Scheduler {
    public: 
        deque<request *> *IO_queue;
        virtual request* get_next_request() = 0;
};

class FIFO : public Scheduler {
    public:
        FIFO() {
            IO_queue = new deque<request *>();
        }
        request* get_next_request() {
            if(IO_queue->empty()) {
                return NULL;
            } else {
                request* req = IO_queue->front();
                IO_queue->pop_front();
                return req;
            }
        }
};

class SSTF : public Scheduler {
    public:
        SSTF() {
            IO_queue = new deque<request *>();
        }
        request* get_next_request() {
            if(IO_queue->empty()) {
                return NULL;
            } else {
                int index = 0;
                for(int i=1; i<IO_queue->size(); i++) {
                    if(abs(IO_queue->at(i)->track-head)<abs(IO_queue->at(index)->track-head)) {
                        index = i;
                    }
                }
                request* req = IO_queue->at(index);
                IO_queue->erase(IO_queue->begin()+index);
                return req;
            }

        }
};

class LOOK : public Scheduler {
    public:
        LOOK() {
            IO_queue = new deque<request *>();
        }
        request* get_next_request() {
            if(IO_queue->empty()) {
                return NULL;
            } else {
                int index = -1;
                for(int i=0; i<IO_queue->size(); i++) {
                    request* req = IO_queue->at(i);
                    if(dir*(req->track-head)<0) {
                        continue;
                    }
                    if(index==-1) {
                        index = i;
                        continue;
                    }
                    if(abs(IO_queue->at(i)->track-head)<abs(IO_queue->at(index)->track-head)) {
                        index = i;
                    }
                }
                if(index==-1) {
                    for(int i=0; i<IO_queue->size(); i++) {
                        request* req = IO_queue->at(i);
                        if(index==-1) {
                            index = i;
                            continue;
                        }
                        if(abs(IO_queue->at(i)->track-head)<abs(IO_queue->at(index)->track-head)) {
                            index = i;
                        }
                    }
                }
                request* req = IO_queue->at(index); 
                IO_queue->erase(IO_queue->begin()+index);
                return req;
                
            }
        }
};

class CLOOK : public Scheduler {
    public:
        CLOOK() {
            IO_queue = new deque<request *>();
        }
        request* get_next_request() {
            if(IO_queue->empty()) {
                return NULL;
            } else {
                int index = -1;
                dir = 1;
                for(int i=0; i<IO_queue->size(); i++) {
                    request* req = IO_queue->at(i);
                    if(dir*(req->track-head)<0) {
                        continue;
                    }
                    if(index==-1) {
                        index = i;
                        continue;
                    }
                    if(abs(IO_queue->at(i)->track-head)<abs(IO_queue->at(index)->track-head)) {
                        index = i;
                    }
                }
                int fakeHead = 0;
                if(index==-1) {
                    for(int i=0; i<IO_queue->size(); i++) {
                        request* req = IO_queue->at(i);
                        if(index==-1) {
                            index = i;
                            continue;
                        }
                        if(abs(IO_queue->at(i)->track-fakeHead)<abs(IO_queue->at(index)->track-fakeHead)) {
                            index = i;
                        }
                    }
                }
                request* req = IO_queue->at(index); 
                IO_queue->erase(IO_queue->begin()+index);
                return req;
                
            }
        }
};

class FLOOK : public Scheduler {
    public:
        deque<request *> *active_queue;
        FLOOK() {
            IO_queue = new deque<request *>();
            active_queue = new deque<request *>();
        }
        request* get_next_request() {
            if(active_queue->empty()) {
                deque<request *> *temp = active_queue;
                active_queue = IO_queue;
                IO_queue = temp;
                if(active_queue->empty()) {
                    return NULL;
                }
            }
            int index = -1;
            for(int i=0; i<active_queue->size(); i++) {
                request* req = active_queue->at(i);
                if(dir*(req->track-head)<0) {
                    continue;
                }
                if(index==-1) {
                    index = i;
                    continue;
                }
                if(abs(active_queue->at(i)->track-head)<abs(active_queue->at(index)->track-head)) {
                    index = i;
                }
            }
            if(index==-1) {
                for(int i=0; i<active_queue->size(); i++) {
                    request* req = active_queue->at(i);
                    if(index==-1) {
                        index = i;
                        continue;
                    }
                    if(abs(active_queue->at(i)->track-head)<abs(active_queue->at(index)->track-head)) {
                        index = i;
                    }
                }
            }
            request* req = active_queue->at(index); 
            active_queue->erase(active_queue->begin()+index);
            return req;
                

        }

};

void printInput() {
    for(int i=0; i<request_l.size(); i++) {
        request req = request_l.at(i);
        printf("%d %d\n", req.arrival_time, req.track);
    }
}

void Simulation() {
    int get_cnt = 0;
    while(true) {
        if(get_cnt<req_total) {
            request* req = &request_l.at(get_cnt);
            if(total_time==req->arrival_time) {
                SCHEDULER->IO_queue->push_back(req);
                get_cnt++;
            }
        }
        if(CURRENT_REQ!=NULL&&CURRENT_REQ->track==head) {
            CURRENT_REQ->end_time = total_time;
            CURRENT_REQ->wait_time = CURRENT_REQ->start_time-CURRENT_REQ->arrival_time;
            CURRENT_REQ->turnaround = CURRENT_REQ->end_time-CURRENT_REQ->arrival_time;
            CURRENT_REQ = NULL;
            req_cnt++;
        }
        if(CURRENT_REQ == NULL) {
            CURRENT_REQ = SCHEDULER->get_next_request();
            if(CURRENT_REQ==NULL&&req_cnt==req_total) {
                break;
            }
            if(CURRENT_REQ!=NULL) {
                CURRENT_REQ->start_time = total_time;
                if(CURRENT_REQ->track>head) {
                    dir = 1;
                } else if(CURRENT_REQ->track<head){
                    dir = -1;
                } else {
                    CURRENT_REQ->end_time = total_time;
                    CURRENT_REQ->wait_time = CURRENT_REQ->start_time-CURRENT_REQ->arrival_time;
                    CURRENT_REQ->turnaround = CURRENT_REQ->end_time-CURRENT_REQ->arrival_time;
                    CURRENT_REQ = NULL;
                    req_cnt++;
                    continue;
                }
            }
        }
        if(CURRENT_REQ!=NULL) {
            head += (1*dir);
            total_move += 1;
        }
        total_time++;
    }
    for(int i=0; i<req_total; i++) {
        request *req = &request_l[i];
        printf("%5d: %5d %5d %5d\n", req->id, req->arrival_time, req->start_time, req->end_time);
    }
}

void Summary() {
    int total_wait_time = 0, total_turnaround = 0, max_waittime = -1;
    for(int i=0; i<req_total; i++) {
        request req = request_l[i];
        total_wait_time += req.wait_time;
        total_turnaround += req.turnaround;
        max_waittime = max(max_waittime, req.wait_time);
    }
    double avg_turnaround = (1.0*total_turnaround)/req_total, avg_waittime = (1.0*total_wait_time)/req_total;
    printf("SUM: %d %d %.2lf %.2lf %d\n", total_time, total_move, avg_turnaround, avg_waittime, max_waittime);
}

int main(int argc, char *argv[]) {
    string line = "#";
    fstream inputFile;
    int c;
    opterr=0;
    while ((c = getopt (argc, argv, "vqfs:")) != -1) {
        switch (c)
        {
        case 'v':
            WHETHER_VERBOSE = true;
            break;
        case 'q':
            WHETHER_QUEUE = true;
            break;
        case 'f':
            WHETHER_FLOOK = true;
            break;
        case 's':
            SCHEDULER_TYPE = optarg;
            break;
        }
    }
    if(optind<argc) {
        INPUT_FILE = argv[optind];
    } else {
        cout << "Missing Input File!" << endl;
        return 0;
    }
    if(strcmp(SCHEDULER_TYPE,"i")==0) {
        SCHEDULER = new FIFO();
    } else if (strcmp(SCHEDULER_TYPE,"j")==0) {
        SCHEDULER = new SSTF();
    } else if (strcmp(SCHEDULER_TYPE,"s")==0) {
        SCHEDULER = new LOOK();
    } else if (strcmp(SCHEDULER_TYPE,"c")==0) {
        SCHEDULER = new CLOOK();
    } else if (strcmp(SCHEDULER_TYPE,"f")==0) {
        SCHEDULER = new FLOOK();
    }

    inputFile.open(INPUT_FILE, fstream::in);
    while(line.at(0)=='#') {
        getline(inputFile, line, '\n');
    }
    while(!inputFile.eof()) {
        if(line=="") {
            break;
        }
        int time_stamp, track;
        sscanf(line.c_str(), "%d %d", &time_stamp, &track);
        request req = {
            .id = req_total++,
            .arrival_time = time_stamp,
            .track = track,
            .end_time = -1,
            .start_time = -1,
            .turnaround = -1,
            .wait_time = -1
        };
        request_l.push_back(req);
        getline(inputFile, line, '\n');
    }
    inputFile.close();
    Simulation();
    Summary();
    // printInput();
}