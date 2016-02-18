#include <iostream>
#include <tiobj.hpp>
#include <stdlib.h>
#include <unistd.h>
#include <tisys.hpp>

using namespace std;


/*void writeDepend(TiObj& obj){
	for (int i=0; i<obj.size(); i++){
		TiObj& node = obj.box[i];
		if ( node.is("Depend") ){
		}
	}
}*/


string getLibraries(TiObj& node){
	string res;
	for (int i=0; i<node.size(); i++){
		TiObj& obj = node.box[i];
		if ( obj.is("Depend") ){
			res += Join("-l%s ").at(obj.atStr("name")).ok;
		}
	}
	return res;
}


int main(int argc, char** argv){
	string nom = getenv("nom");
	chdir(nom.c_str());

	TiObj obj;
	obj.loadFile(".sysobj.ti");
	
	string header;
	string body;
	string mkdir;
	string install;
	
	string class_name = obj.atStr("name");
	string bin_url = Join("./%s/bin").at(class_name).ok;
	for (int i=0; i<obj.size(); i++){
		TiObj& node = obj.box[i];
		if ( node.is("Cpp") ){
			string src = Join("../src/%s").at(node.atStr("src")).ok;
			string dst = node.atStr("name");
			
			header += dst + " ";
			body += Join("%s: %s\n").at(dst).at(src).ok;
			body += Join("\tg++ '%s' -o '%s' %s\n\n").at(src).at(dst).at(getLibraries(node)).ok;
			
			install += Join("\tcp '%s' '.builded/bin/%s'\n").at(dst).at(dst).ok;
		} else if ( node.is("Bash") ){
	
			string src = Join("../src/%s").at(node.atStr("src")).ok;
			string dst = Join(".builded/bin/%s").at(node.atStr("name")).ok;
			install += Join("\tcp '%s' '%s'\n").at(src).at(dst).ok;
			install += Join("\tchmod 755 '%s'\n").at(dst).ok;
		} else if ( node.is("Package") ){
			string name = node.atStr("name");
			install += Join("\tmkdir -p '.builded/bin/%s'\n").at(name).ok;
			mkdir += Join("\tmkdir -p '%s'\n").at(name).ok;
		} else if ( node.is("Data") ){
			string src = node.atStr("src");
			string dst = node.atStr("name");
			install += Join("\tcp -rf '../%s' '.builded/%s'\n").at(src).at(dst).ok;
		}
	}
	
	cout << "all: __mkdir " << header << endl << endl;
	cout << body << endl;
	
	cout << "install:\n";
	cout << "\tmkdir -p .builded/bin\n";
	cout << install << endl;
	
	cout << "__mkdir:\n";
	cout << mkdir;
	
	
	return 0;
}
