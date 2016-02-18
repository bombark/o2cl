/*====================================- HEADER -========================================*/

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <tiobj.hpp>
#include <unistd.h>
#include <vector>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

using namespace std;


/*--------------------------------------------------------------------------------------*/


class ProcessPkg {
	//vector< FILE* > process;
	vector< pid_t > process;

  public:
	void exec(TiObj& node);
	void kill(int i);
};



void ProcessPkg::exec(TiObj& node){
	
	pid_t childPID = fork();
	if(childPID >= 0){
		if(childPID == 0){
			// Set the parameters
			string classe = node.atStr("class");
			string name   = node.atStr("name", classe);

			string cmd = Join("./bin/%s").at(classe).ok;
			string url = Join("./objs/%s").at(name).ok;

			node.set("url", url);


			mkdir(url.c_str(), 0755);
			setenv("params",node.encode().c_str(), true);


			// Execute the command bin/classe and show the stdout
			char buffer[1024];
			string log = Join("[%s]: ").at(classe).ok;
			FILE* fp = popen( cmd.c_str(), "r" );
			while ( fgets(buffer,1024,fp) ){
				cout << log << buffer;
			}
			cout << log << "Finishing" << endl;
			exit(0);

		} else {
			cout << "Init process: " << node.atStr("class") << endl;
			//this->process.push_back(childPID);

		}
	} else {
		cerr << "Fork failed, quitting!!!!!!\n";
	}

}









void ProcessPkg::kill(int i){
	cout << "Finalizing the process "<< i << endl; 
	this->process.erase(this->process.begin()+i);
}


/*=====================================- MAIN -=========================================*/


int main(int argc, char ** argv) {
	TiObj params;
	params.loadText( getenv("params") );
	string nom = params.atStr("nom");
	chdir( nom.c_str() );


	mkdir("/run/shm/objs-robot", 0755);
	symlink("/run/shm/objs-robot", "objs");

	TiObj robot;
	robot.loadFile(".sysobj.ti");

	cout << "Initializing:\n";

	ProcessPkg processpkg;
	for (int i=0; i<robot.size(); i++){
		TiObj& node = robot.box[i];
		processpkg.exec(node);
	}

	while(1){
		sleep(1);
	}
	return 0;
}

/*--------------------------------------------------------------------------------------*/
