#include <iostream>
#include <tiobj.hpp>
#include <vector>
#include <stdio.h>
#include <stdlib.h>

using namespace std;


class Elisyum{
	TiObj  db;
	vector<TiObj*> stack;
	std::string root;

  public:
	Elisyum() {
		
	}

	void create(std::string filename){
		cerr << "Compiling " << filename << endl;

		TiObj objs;
		objs.loadFile(filename);

		if ( objs.has("template") ){
			db.loadFile( objs.atStr("template") );
			if ( db.is("ERROR") ){
				cerr << "Template não encontrado\n";
				return;
			}
		} else {
			cerr << "Nao existe template configurado\n";
			return;
		}

		this->root = objs.atStr("root",".");

		int pos = filename.find_last_of('.');
		string outfile = Join("%s.html").at(filename.substr(0,pos)).ok;
		freopen (outfile.c_str(),"w",stdout);
		cout << "<html><head>\n";
		cout << "\t<meta charset=\"utf-8\">\n";
		for (int i=0; i<objs.size(); i++){
			TiObj& node = objs.box[i];
			if ( node.is("Css") ){
				cout << Join("\t<link href=\"%s\" rel=\"stylesheet\">\n").at(node.atStr("url")).ok;
			}/* else if ( node.is("Js") ){
				cout << Join("\t<link href=\"%s\" rel=\"stylesheet\">\n").at(node.atStr("url")).ok();
			}*/
		}
		if ( objs.has("title") ){
			cout << Join("\t<title>%s</title>").at(objs.atStr("title")).ok;
		}

		cout << "</head><body>\n";
		for (int i=0; i<objs.size(); i++){
			this->process( objs.box[i] );
		}
		cout << "</body></html>\n";

		fclose(stdout);
	}



	void process(TiObj& obj){
		bool error = false;
		if ( obj.classe == "" ){
			if ( this->stack.size() == 0 ){
				if ( obj.has("html") ){
					cout << obj.atStr("html");
					return;
				}
			} else {
				TiObj& parent = *this->stack[ this->stack.size()-1 ];
				if ( parent.hasnt("default") ){
					cerr << "Default do item nao encontrado\n";
					error = true;
				}
			}
		} else if ( obj.classe != "Css" ){
			if ( this->db.hasnt( obj.classe ) ){
				cerr << Join("Item nao encontrado %s\n").at(obj.classe).ok;
				error = true;
			}
		}
		if ( error )
			return;


		if ( obj.is("Package") ){
			this->stack.push_back(   &this->db.atObj(obj.classe)   );
			cout << this->parse( db.atObj( obj.classe ).atStr("ini"), obj);
			for (int i=0; i<obj.size(); i++){
				this->process( obj.box[i] );
			}
			cout << this->parse( db.atObj( obj.classe ).atStr("end"), obj);
			this->stack.pop_back();
		} else {
			if ( obj.classe == "" ){
				TiObj& parent = *this->stack[ this->stack.size()-1 ];
				cout << this->parse( parent.atStr("default"), obj);
			} else {
				TiVar& var = db.at(obj.classe);
				if ( var.isObj() )
					cout << this->parse(var.atObj().atStr("html"), obj);
				else if ( var.isStr() )
					cout << this->parse(var.atStr(), obj);
			}
		}

	}




	std::string parse(std::string html, TiObj& data){
		string out = "";
		string var;
		string def;
		bool isvar = false;

		for (int i=0; i<html.size(); i++){
			char c = html[i];

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
					
					while ( i<html.size() ){
						char c = html[i];
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
					if ( var == "root" ){
						out += this->root;
					} else {
						if ( data.has(var) ){
							out += data.atStr(var);
						}
					}
					out += c;
					var = "";
				}
			}

		}
		return out;
	}

};






std::vector<std::string> explode(char qry, std::string text){
	std::vector<std::string> out;
	string buffer;
	for (int i=0; i<text.size(); i++){
		char c = text[i];
		if ( c == qry ){
			if ( buffer.size() != 0 ){
				out.push_back(buffer);
				buffer = "";
			}
		} else {
			buffer += c;
		}
	}
	if ( buffer.size() != 0 )
		out.push_back(buffer);
	return out;
}




int main(int argc, char** argv){
	string _akk = getenv("_Akk");
	std::vector<std::string> akk = explode(',', _akk);
	for (int i=0; i<akk.size(); i++){
		Elisyum el;
		el.create(akk[i]);
	}
	return 0;
}
