#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include "Commands.h"

using namespace std;
const std::string WHITESPACE = " \n\r\t\f\v";

#if 0
#define FUNC_ENTRY()  \
  cerr << __PRETTY_FUNCTION__ << " --> " << endl;

#define FUNC_EXIT()  \
  cerr << __PRETTY_FUNCTION__ << " <-- " << endl;
#else
#define FUNC_ENTRY()
#define FUNC_EXIT()
#endif

#define DEBUG_PRINT cerr << "DEBUG: "

#define EXEC(path, arg) \
  execvp((path), (arg));

string _ltrim(const std::string& s)
{
    size_t start = s.find_first_not_of(WHITESPACE);
    return (start == std::string::npos) ? "" : s.substr(start);
}

string _rtrim(const std::string& s)
{
    size_t end = s.find_last_not_of(WHITESPACE);
    return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

string _trim(const std::string& s)
{
    return _rtrim(_ltrim(s));
}

int _parseCommandLine(const char* cmd_line, char** args) {
    FUNC_ENTRY()
    int i = 0;
    std::istringstream iss(_trim(string(cmd_line)).c_str());
    for(std::string s; iss >> s; ) {
        args[i] = (char*)malloc(s.length()+1);
        memset(args[i], 0, s.length()+1);
        strcpy(args[i], s.c_str());
        args[++i] = NULL;
    }
    return i;

    FUNC_EXIT()
}

bool _isBackgroundComamnd(const char* cmd_line) {
    const string str(cmd_line);
    return str[str.find_last_not_of(WHITESPACE)] == '&';
}

void _removeBackgroundSign(char* cmd_line) {
    const string str(cmd_line);
    // find last character other than spaces
    unsigned int idx = str.find_last_not_of(WHITESPACE);
    // if all characters are spaces then return
    if (idx == string::npos) {
        return;
    }
    // if the command line does not end with & then return
    if (cmd_line[idx] != '&') {
        return;
    }
    // replace the & (background sign) with space and then remove all tailing spaces.
    cmd_line[idx] = ' ';
    // truncate the command line string up to the last non-space character
    cmd_line[str.find_last_not_of(WHITESPACE, idx) + 1] = 0;
}

// TODO: Add your implementation for classes in Commands.h

///command///
Command::Command(const char* cmd_line): pid(-1) {

    if (_isBackgroundComamnd(cmd_line))
        state = Background;
    else
        state = Foregroung;

    this->cmd_line = (char *) malloc(sizeof(char) * strlen(cmd_line) + 1);
    strcpy(this->cmd_line, cmd_line);
}

Command::~Command() {
    free(cmd_line);
}

char * Command::get_cmd_line() {
    return this->cmd_line;
}

void Command::set_pid(pid_t new_pid) {
    this->pid = new_pid;
    return;
}

pid_t Command::get_pid() {
    return this->pid;
}

void Command::set_state(State new_state) {
    this->state = new_state;
    return;
}

State Command::get_state() {
    return this->state;
}

///showpid///
void ShowPidCommand::execute() {
    cout << "smash pid is " << getpid() << endl;
}

///pwd///
void GetCurrDirCommand::execute() {
    char cwd[1024];
    if(getcwd(cwd, sizeof(cwd)) == NULL){
        perror("smash error: getcwd failed");
    }
    cout << cwd <<endl;
    return;
}

SmallShell::SmallShell() {
// TODO: add your implementation
}


/**
* Creates and returns a pointer to Command class which matches the given command line (cmd_line)
*/
Command * SmallShell::CreateCommand(const char* cmd_line) {
    // For example:
/*
  string cmd_s = string(cmd_line);
  if (cmd_s.find("pwd") == 0) {
    return new GetCurrDirCommand(cmd_line);
  }
  else if ...
  .....
  else {
    return new ExternalCommand(cmd_line);
  }
  */
    return nullptr;
}

void SmallShell::executeCommand(const char *cmd_line) {
    // TODO: Add your implementation here
    // for example:
    // Command* cmd = CreateCommand(cmd_line);
    // cmd->execute();
    // Please note that you must fork smash process for some commands (e.g., external commands....)
}


//////jobs//////
void JobsList::addJob(Command *cmd, bool isStopped) {
    if (!cmd) return;
    removeFinishedJobs();

    if (isStopped){
        cmd->set_state(Stopped);
    }
    else{
        cmd->set_state(Background);
    }

    int new_id = ((this->jobs).back().get_job_id()) + 1;
    JobEntry new_job = JobEntry(cmd,new_id);
    this->jobs.push_back(new_job);
}

void JobsList::printJobsList() {
    removeFinishedJobs();

    time_t time2;
    if(time(&time2) == (time_t)-1){
        perror("smash error: time failed");
    }

    for(size_t i = 0; i < (this->jobs).size(); ++i){
        double elapsed_time = difftime(time2, jobs[i].get_time());

        if (jobs[i].get_cmd()->get_state() == Stopped){

            cout << "[" << jobs[i].get_job_id() << "] "<< jobs[i].get_cmd()->get_cmd_line()
                 << " : " << jobs[i].get_cmd()->get_pid() << " " << elapsed_time << "secs (stopped)" << endl;
        }
        else{
            cout << "[" << jobs[i].get_job_id() << "] "<< jobs[i].get_cmd()->get_cmd_line()
                 << " : " << jobs[i].get_cmd()->get_pid() << " " << elapsed_time << " secs" << endl;
        }
    }
}

JobsList::JobEntry* JobsList::getJobById(int jobId) {
    removeFinishedJobs();

    for (size_t i = 0; i < (this->jobs).size(); ++i) {
        if (jobs[i].get_job_id() == jobId){
            return &(jobs[i]);
        }
    }
}

void JobsList::removeJobById(int jobId) {
    removeFinishedJobs();

    for (size_t i = 0; i < (this->jobs).size(); ++i) {
        if (jobs[i].get_job_id() == jobId){
            jobs.erase(jobs.begin() + i);
            return;
        }
    }
}

JobsList::JobEntry* JobsList::getLastJob(int *lastJobId) {
    removeFinishedJobs();

    *lastJobId = jobs.back().get_job_id();
    return &(jobs.back());
}

JobsList::JobEntry* JobsList::getLastStoppedJob(int *jobId) {
    removeFinishedJobs();

    for (size_t i = (this->jobs).size()-1; i >= 0; --i) {
        if (jobs[i].get_cmd()->get_state() == Stopped){
            *jobId = jobs[i].get_job_id();
            return  &(jobs[i]);
        }
    }
}

bool JobsList::is_job_exist(int id) {
    removeFinishedJobs();

    for (size_t i = 0; i < (this->jobs).size(); ++i) {
        if (jobs[i].get_job_id() == id) {
            return true;
        }
    }

    return false;
}

void JobsList::killAllJobs() {

    removeFinishedJobs();

    for (size_t i = 0; i < (this->jobs).size(); ++i) {
        int ret = kill(jobs[i].get_cmd()->get_pid(), SIGKILL);
        if (ret != 0) {
            perror("smash error: kill failed");
        }
    }
}
