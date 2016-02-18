#include <iostream>
#include <tiobj.hpp>
#include <stdlib.h>
#include <unistd.h>

using namespace std;


/*void writeDepend(TiObj& obj){
	for (int i=0; i<obj.size(); i++){
		TiObj& node = obj.box[i];
		if ( node.is("Depend") ){
		}
	}
}*/



int main(int argc, char** argv){
	string nom = getenv("nom");
	chdir(nom.c_str());

	TiObj obj;
	obj.loadFile(".sysobj.ti");
	
	
	
	cout << "cmake_minimum_required (VERSION 2.6)" << endl;
	cout << Join("project (%s)").at(obj.atStr("name")).ok << endl;


	for (int i=0; i<obj.size(); i++){
		TiObj& node = obj.box[i];
		if ( node.is("App") ){
			string name = node.atStr("name");
			cout << Join("add_executable (%s %s)").at(name).at(node.atStr("src")).ok << endl;
			
			
			
			
			
			cout << Join("install(TARGETS %s DESTINATION bin)").at(name).ok << endl;
		} else if ( node.is("Lib") ){
		
		}
	}
	return 0;
}
