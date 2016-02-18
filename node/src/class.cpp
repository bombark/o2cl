#include<iostream>
#include<tiobj.hpp>
#include<tisys.hpp>
#include<stdlib.h>

using namespace std;


int main(){
	Filesystem fs;
	string nom = getenv("nom");
	string classe = fs.node_type(nom);
	cout << classe << endl;
	return 0;
}
