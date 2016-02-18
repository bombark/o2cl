#include<iostream>
#include<tiobj.hpp>
#include<stdio.h>
#include<stdlib.h>

using namespace std;



std::string parse(std::string text, TiObj& data){
	string out = "";
	string var;
	string def;
	bool isvar = false;

	for (int i=0; i<text.size(); i++){
		char c = text[i];

		if ( isvar == false ){
			if ( c == '@' ){
				isvar = true;
			} else {
				out += c;
			}
		} else {
			if ( isalnum(c) || c==':' || c=='_' ){
				var += c;
			} else if ( c == '=' ){
				isvar = false;

				i += 1;
				
				while ( i<text.size() ){
					char c = text[i];
					if ( c == ';' )
						break;
					def += c;
					i+=1;
				}

				
				if ( data.has(var) ){
					out += data.atStr(var);
				} else {
					out += def;
				}
			
				def = "";
				var = "";
			} else {
				isvar = false;
				
				if ( data.has(var) ){
					out += data.atStr(var);
				}
				
				out += c;
				var = "";
			}
		}

	}
	return out;
}




int main(){
	string nom = getenv("nom");
	string to_url = getenv("_Akk");


	TiObj data;
	data.loadFile(nom);

	FILE* fd = fopen(to_url.c_str(), "r");
	if ( !fd )
		return 1;
	string to_text;
	while ( !feof(fd) ){
		char buffer[1024];
		buffer[0] = '\0';
		fgets(buffer,1024,fd);
		to_text += buffer;
	}
	fclose(fd);



	cout << parse(to_text, data);
	return 0;
}
