#include <iostream>
#include <tiobj.hpp>
#include <tisys.hpp>
#include <tiarg.hpp>
#include <stdlib.h>

using namespace std;


bool isFolder(string url){
	struct stat n_stat;
	if ( stat(url.c_str(), &n_stat) != -1 ){
		return (S_ISDIR(n_stat.st_mode));
	} else {
		return false;
	}
}


int main(int argc, char** argv){
	if ( argc < 2 ){
		return 0;
	}

	string class_name = path_last(argv[0]);
	
	string nom;
	string method = argv[1];
	int arg_init = 1;
	if ( method[0] == '@' ){
		arg_init = 2;
		nom = method.substr(1);
		if ( argc > 2 )
			method = argv[2];
		else
			return 0;
	} else {
		nom = path_context(class_name);
		if ( nom == "" )
			nom = ".";
	}



	TiObj params;
	getArgs(params, argc-arg_init, argv+arg_init);



	string class_url  = Join("/o2cl/class/%s").at(class_name).ok;
	string cmd = Join("%s/bin/%s").at(class_url).at(method).ok;
	
	

	
	
	/*COLOCAR um defaulf
	if ( isFolder(cmd) ){
		string akk = params.atStr("_Akk");
		size_t pos = akk.find_last_of(":");
		cmd = Join("%s/%s").at(cmd).at(akk.substr(0,pos)).ok;
		params.set("_Akk", akk.substr(pos+1));
	}*/
	params.set("nom",nom);



	for (int i=0; i<params.length(); i++){
		setenv(params[i].name.c_str(), params[i].atStr().c_str(), true);
	}
	setenv("params", params.encode().c_str(), true);

	setenv("class_name", class_name.c_str(), true);
	setenv("class_url", class_url.c_str(), true);


	return system(cmd.c_str());
}
