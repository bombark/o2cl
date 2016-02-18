#include<iostream>
#include<tiobj.hpp>
#include<tisys.hpp>
#include<stdlib.h>

using namespace std;


int main(){
	Filesystem fs;
	
	string nom = getenv("nom");
	string type = fs.node_type(nom);

	TiObj tmp;
	string akk = getenv("_Akk");
	tmp.classe = type;
	
	if ( !tmp.is(akk) )
		cout << "y\n";

	return 0;
}
