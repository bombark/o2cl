/*====================================- HEADER -========================================*/

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <tiobj.hpp>
#include <unistd.h>
#include <vector>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <time.h> 

#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ioctl.h>

using namespace std;


/*--------------------------------------------------------------------------------------*/



/*====================================- Process -=======================================*/

class Process {
  public:
	string classe;
	string name;
	string url;
	TiObj* params;
	
	pid_t pid;
	int   fd_read[2];
	int   fd_write[2];
	
	char buffer[512];
	int clock;
	
	uint nread;
	
	//clock_t t_ini;
	struct timeval t_ini;
	
	double time;
	
	
	fd_set readfds;
	struct timeval tv;
	
	int pmutex, pmask;
	std::vector< Process* > activates;
	std::vector< uint > activates_pos;
	
	std::vector< Process* > depends;
	
	
	Process(std::string classe, std::string name, TiObj& params);

	
	char* recv();
	uint  send(std::string buf);
	
	
	void exec(uint time_wait=20000);
	
	void addRecv( Process* from, std::string as_name );
	
	void startStep();
	void endStep();
	
	void  close();
	void kill();
};

/*--------------------------------------------------------------------------------------*/



/*==================================- ProcessPkg -======================================*/


class ProcessPkg {
	int time_sleep;
	int time_procwait;
	vector< Process* > process;
	bool debug;


  public:
	ProcessPkg();
	~ProcessPkg();
	void exec(TiObj& node);
	Process* find(std::string name);
	void close(int i);
	void rm(int pid);

	void check();
	void wait();
	void show();
};


/*--------------------------------------------------------------------------------------*/









/*====================================- Process -=======================================*/


Process::Process(std::string classe, std::string name, TiObj& params){
	this->classe = classe;
	this->name = name;
	tv.tv_sec =  0;
	tv.tv_usec = 0;
	this->clock = 0;
	this->pmutex = 0;
	this->pmask  = 0;
	this->url = Join("./objs/%s").at(name).ok;
	mkdir(this->url.c_str(), 0755);
	this->params = &params;
	
	gettimeofday(&this->t_ini, NULL);
	this->fd_read[0] = -1;
	this->fd_write[1] = -1;
}

char* Process::recv(){
	FD_ZERO(&readfds);
	FD_SET(this->fd_read[0], &readfds);
	this->nread = 0;

	//flush(this->fd[0]);
	//ioctl(this->fd[0], BLKFLSBUF, 0);
	
	int num_readable = select(this->fd_read[0]+1, &readfds, NULL, NULL, &tv);
	
	if ( num_readable > 0 ){

		this->nread = ::read(this->fd_read[0], buffer, sizeof(buffer)-1);
		if ( this->nread == 0 )
			this->close();
		buffer[this->nread] = '\0';
	} else if ( num_readable == 0 ){
		// wait
		
		buffer[0] = '\0';
	} else 
		// kill the process
		buffer[0] = '\0';
	return buffer;
}

uint Process::send(std::string buffer){
	int wrote = write(this->fd_write[1], buffer.c_str(), buffer.size());
	//fsync(this->fd_write[1]);
	//fdatasync(this->fd_write[1]);
	return wrote;
}






void Process::exec(uint time_wait){
	this->tv.tv_usec = time_wait;
	
	pipe(this->fd_read);
	pipe(this->fd_write);
	
	pid_t childPID = fork();
	if(childPID >= 0){
		if(childPID == 0){
			//Close up input
			::close(this->fd_read[0]);
			::close(this->fd_write[1]);

			setenv("params",this->params->encode().c_str(), true);
			chdir(this->url.c_str());

			// Duplicate the input side of pipe to stdin
			dup2 (this->fd_write[0], STDIN_FILENO);
			dup2 (this->fd_read[1], STDOUT_FILENO);

			::close(this->fd_write[0]);
			::close(this->fd_read[1]);


			//setvbuf(stdout, NULL, _IONBF, 0);
			//setvbuf(stdin, NULL, _IONBF, 0);


			string cmd = classe;
			execlp(cmd.c_str(), cmd.c_str(), NULL);
			printf("\nError: Could not execute function %s\n", cmd.c_str());
			_exit(0);

		} else {
			::close(this->fd_read[1]);
			::close(this->fd_write[0]);
			
			this->pid = childPID;
			cout << "Init process: " << this->name << " : " << this->pid << endl;
		}
	} else {
		cerr << "Fork failed, quitting!!!!!!\n";
	}


}



void Process::addRecv( Process* from, std::string as_name ){
	// Create the symbolic link
	string src  = Join("../%s").at( from->name ).ok;
	string link = Join("%s/%s").at( this->url ).at( as_name ).ok;
	symlink(src.c_str(), link.c_str());

	// Add Process to activate in the from process
	uint pos = 1 << this->depends.size(); 
	from->activates_pos.push_back( pos );
	from->activates.push_back( this );
	
	// Add a new depends in the process
	this->depends.push_back( from );
	this->pmask = (1 << this->depends.size()) - 1;
}


void Process::startStep(){
	//cout << "start " << this->name << endl;
	this->pmutex = 0;

	//this->t_ini = ::clock();
	gettimeofday(&this->t_ini, NULL);


	// Necessary send a #start
	// proc.send("#start\n");
}



void Process::endStep(){
	//clock_t t_end = ::clock();
	//this->time = (double) (t_end-this->t_ini) / CLOCKS_PER_SEC * 1000.0;
	
	struct timeval t_end;
	gettimeofday(&t_end, NULL);
	this->time = (1000.0 * (t_end.tv_sec - t_ini.tv_sec) + (t_end.tv_usec - t_ini.tv_usec) / 1000.0);


	if ( this->depends.size() == 0 ){
		this->startStep();
	}
	
	for (int i=0; i<activates.size(); i++){
		Process& proc = *this->activates[i];
		proc.pmutex |= this->activates_pos[i];
		
		if ( (proc.pmutex&proc.pmask) == proc.pmask ){
			proc.startStep();
			proc.send("#start\n");
		}
	}

}



void Process::close(){
	if ( this->fd_read[0] != -1 )
		::close(this->fd_read[0]);
	if ( this->fd_write[1] != -1 )
		::close(this->fd_write[1]);
	this->fd_read[0] = -1;
	this->fd_write[1] = -1;
}

void Process::kill(){
	int ret = ::kill(this->pid, SIGINT); //SIGINT);  //SIGHUP);
	cout << "send signal : " << ret << endl;
}

/*--------------------------------------------------------------------------------------*/





/*==================================- ProcessPkg -======================================*/

ProcessPkg::ProcessPkg(){
	this->time_procwait = 1000;
	this->time_sleep = 10000;
	this->debug = false;
}

ProcessPkg::~ProcessPkg(){
	for (int i=0; i<process.size(); i++){
		Process& proc = *this->process[i];
		cout << "Finalizing the process "<< proc.name << endl;
		proc.close();
		proc.kill();
	}

	if ( process.size() > 0 ){
		cout << "Waiting...\n";
		sleep(1);
	}
}


void ProcessPkg::exec(TiObj& robot){
	if ( robot.has("debug") ){
		this->debug = true;
	}


	for (int i=0; i<robot.size(); i++){
		TiObj& node = robot.box[i];
		Process* proc = new Process(node.atStr("class"), node.atStr("name"), node);
		this->process.push_back( proc );
	}


	for (int i=0; i<robot.size(); i++){
		TiObj& task   = robot.box[i];
		Process& proc = *this->process[i];

		for (int j=0; j<task.size(); j++){
			TiObj& node = task.box[j];
			if ( node.is("Recv") ){
				Process* proc_from = this->find( node.atStr("from") );
				if ( proc_from == NULL ){
					cerr << "[Error]: Process " << node.atStr("from") << " don't exist\n";
					exit(1);
				}
				proc.addRecv( proc_from, node.atStr("as",proc.name) );
			}
		}
	}


	




	for (int i=0; i<process.size(); i++){
		Process& proc = *process[i];
		cout << proc.name << " " << proc.activates.size() << " " << proc.depends.size() << endl;
	}



	//this->time_sleep = 5000;// - this->time_procwait * this->process.size();
	//if ( this->time_sleep < 0 )
	//	this->time_sleep = 0;

	for (int i=0; i<process.size(); i++){
		process[i]->exec(this->time_procwait);
	}
}

Process* ProcessPkg::find(std::string name){
	for (int i=0; i<process.size(); i++){
		if ( name == process[i]->name )
			return process[i];
	}
	return NULL;
}



void ProcessPkg::check(){
	//cout << "Lendo\n";

	bool updated = false;
	for (int i=0; i<process.size(); i++){
		Process& proc = *this->process[i];
		
		char* buf = proc.recv();
		
		
		
		if ( proc.nread > 0 ){
			//cout << Join("[%s]: %s\n").at(proc.name).at(buf).ok;
			if ( buf[0] == '#' ){
				proc.clock += 1;
				proc.endStep();
				updated = true;
			} else {
				printf("%s", buf);
			}
		}
	}
	
	if ( this->debug && updated )
		this->show();
}

void ProcessPkg::wait(){
	usleep( this->time_sleep );
}



void ProcessPkg::show(){
	for (int i=0; i<process.size(); i++){
		Process& proc = *this->process[i];
		cout << Join("%s (%d - %d) : %d < %d\n").at(proc.name).at(proc.clock).at(proc.time).at(proc.pmutex).at((int)proc.pmask).ok;
	}
}


void ProcessPkg::rm(int pid){
	int i=0;
	Process* proc = NULL;
	for (; i<process.size(); i++){
		proc = this->process[i];
		if ( proc->pid == pid )
			break;
	}
	if ( proc ){
		proc->close();
		delete proc;
		this->process.erase( this->process.begin()+i );
	}
}


/*--------------------------------------------------------------------------------------*/



/*=====================================- MAIN -=========================================*/

ProcessPkg G_processpkg;

void intHandler(int dummy){
	cout << "Signal " << dummy << endl;
	exit(0);
}

void intChild(int sig_num){
	cout << "Child Signal " << sig_num << endl;
	int signal;
	int pid = wait(&signal);
	G_processpkg.rm(pid);
}




int main(int argc, char ** argv) {

	TiObj params;
	params.loadText( getenv("params") );
	string nom = params.atStr("nom");
	chdir( nom.c_str() );

	setenv( "robot_url", nom.c_str(), true );
	
	

	// Update the environment variable PATH used in ProcessPkg::exec
	string path = getenv("PATH");
	path = Join("%s/bin:%s").at(nom).at(path).ok;
	setenv("PATH",path.c_str(), true);


	mkdir("/run/shm/objs-robot", 0755);
	symlink("/run/shm/objs-robot", "objs");

	TiObj robot;
	robot.loadFile(".sysobj.ti");

	cout << "Initializing!\n";


	G_processpkg.exec(robot);

	signal( SIGINT, intHandler);
	signal(SIGCHLD, intChild); 
	
	
	while(1){
		G_processpkg.check();
		G_processpkg.wait();
	}
	return 0;
}

/*--------------------------------------------------------------------------------------*/
