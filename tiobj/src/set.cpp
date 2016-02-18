#include<iostream>
#include<tiobj.hpp>
#include<stdio.h>
#include<stdlib.h>

using namespace std;


int main(){
	TiObj params;
	params.loadText( getenv("params") );

	if ( params.hasnt("nom") ){
		cerr << "Need to set nomative parameter\n";
		return 1;
	}
	if ( params.hasnt("_Akk") ){
		cerr << "Need to set acusative parameter\n";
		return 1;
	}


	string nom = params.atStr("nom");
	string _akk = params.atStr("_Akk");

	TiObj akk;
	akk.loadText(_akk);
	
	if ( akk.is("Error") ){
		cerr << "Syntax error in the acusative parameter\n";
		return 1;
	}

	TiObj buf;

	buf.loadFile(nom);
	if ( buf.is("Error") ){
		cerr << buf;
		return 1;
	}

	for (int i=0; i<akk.length(); i++){
		buf.set( akk[i].name, akk[i].atStr() );
	}

	for (int i=0; i<akk.size(); i++){
		buf.box += akk.box[i];
	}

	if ( params.has("to") )
		buf.saveFile( params.atStr("to") );
	else
		cout << buf;


	return 0;
}
