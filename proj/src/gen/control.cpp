#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <tiobj.hpp>

using namespace std;


int main(int argc, char** argv){
	char* nom = getenv("nom");
	chdir(nom);
	
	TiObj obj;
	obj.loadFile(".sysobj.ti");
	


	TiObj buf;
	for (int i=0; i<obj.size(); i++){
		TiObj& node = obj.box[i];
		if ( node.is("Depend") )
			buf.box += node;
	}
	
	string depends;
	if ( buf.size() > 0 ){
		depends = buf.box[0].atStr("name");
		for (int i=1; i<buf.size(); i++){
			depends += ","+buf.box[i].atStr("name");
		}
	}

	// o nome deve ter apenas letra minusculas
	cout << "Package: " << obj.atStr("name") << endl;
	cout << "Version: " << obj.atStr("version","0.0.1") << endl;
	cout << "Architecture: " << obj.atStr("arch","amd64") << endl;
	cout << "Maintainer: " << obj.atStr("maintainer","felipe") << endl;

	cout << "Section:" << obj.atStr("section","extra") << endl;
	cout << "Priority:" << obj.atStr("priority","optional") << endl;
	
	if ( depends != "" )
		cout << "Depends:" << depends << endl;
	
	cout << "Description:" << endl;
	cout << obj.atStr("description") << endl;
	
	
	
	
	return 0;
}
