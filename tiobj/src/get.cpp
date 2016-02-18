#include<iostream>
#include<tiobj.hpp>
#include<stdio.h>
#include<stdlib.h>

using namespace std;


int main(){
	string nom = getenv("nom");
	string akk = getenv("_Akk");

	TiObj buf;
	if ( nom == "." ){
		buf.loadFile(stdin);
	} else {
		buf.loadFile(nom);
		if ( buf.is("ERROR") ){
			cerr << buf;
			return 1;
		}
	}
	
	
	cout << buf.atStr(akk) << endl;
	return 0;
}
