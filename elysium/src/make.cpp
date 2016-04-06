#include <iostream>
#include <tiobj.hpp>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <functional>
#include <cassert>

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
			TiVar& var_tmpl = objs["template"];
			if ( var_tmpl.isObj() ){
				this->db = var_tmpl.atObj();
			} else {
				db.loadFile( objs.atStr("template") );
				if ( db.is("Error") ){
					cerr << objs;
					return;
				}
			}
		} else {
			if ( objs.is("Error") ){
				cerr << objs;
			} else 
				cerr << "Nao existe template configurado\n";
			return;
		}

		this->root = objs.atStr("root",".");

		// Reopen cout
		int pos = filename.find_last_of('.');
		string outfile = Join("%s.html").at(filename.substr(0,pos)).ok;
		freopen (outfile.c_str(),"w",stdout);

		// Create the response
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

		// Close the file
		fclose(stdout);
	}


	std::string getHtmlFromClass(std::string class_name){
		if ( class_name == "Css" )
			return "";

		TiVar& var = db.at(class_name);
		if ( var.isObj() )
			return var.atObj().atStr("html");
		else if ( var.isStr() )
			return var.atStr();
		else{
			cerr << "   Error in " << class_name << endl;
			return "";
		}
	}

	inline TiObj& getTemplate( TiObj& node ){
		TiVar& var = db.at(node.classe);
		return *this->getObject(var);
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

		// Get Template
		TiObj* tempalte;
		if ( obj.classe == "" ){
			if ( this->stack.size() > 0 ){
				TiObj* parent = this->stack.back();
				if ( parent->has("default") ){
					tempalte = this->getObject(parent->at("default"));
				}
			} else
				cerr << "Pilha desbalanceada\n";

		} else
			tempalte = &getTemplate(obj); 
		assert(tempalte!=nullptr);

		// Process the template with the variable
		if ( tempalte->has("ini") || tempalte->has("end") ){
			this->stack.push_back(   &this->db.atObj(obj.classe)   );
			std::string ini = tempalte->atStr("ini");
			std::string end = tempalte->atStr("end");
			cout << this->parseTmpl(ini,obj);
			for (int i=0; i<obj.size(); i++){
				this->process( obj.box[i] );
			}
			cout << this->parseTmpl(end,obj);
			this->stack.pop_back();
		} else {
			cout << this->parseTmpl(tempalte->atStr("txt"),obj);
		}

	}



	// text= "<p> @title(default=10)"
	// node= "Title{title='Um novo começo';text='felipe'}"
	std::string parseTmpl(std::string text, TiObj& node){
		// decl= "name{default=10}"
		auto func = [](Elisyum& el, TiObj& decl, TiObj& var) -> std::string {
			string res;
			string field = decl.classe;
			if ( field == "root" ){
				return el.root;
			}
			
			if ( var.has(field) ){
				res += decl.atStr("ini");
				res += var.atStr( field );
				res += decl.atStr("end");
			} else {
				if ( decl.has("default") ){
					res += decl.atStr("ini");
					res += decl.atStr("default");
					res += decl.atStr("end");
				}
			}
			return res;
		};
		return this->parseText('@', text, node, func);
	}





	std::string parseVar(std::string text, TiObj& node){
		// decl= "name{default=10}"
		auto func = [](Elisyum& el, TiObj& decl, TiObj& var) -> std::string {
			string res;
			string field = decl.classe;
			if ( var.has(field) ){
				res += decl.atStr("ini");
				res += var.atStr( field );
				res += decl.atStr("end");
			} else {
				if ( decl.has("default") ){
					res += decl.atStr("ini");
					res += decl.atStr("default");
					res += decl.atStr("end");
				}
			}
			return res;
		};
		return this->parseText('$', text, node, func);
	}





	std::string parseText(
		char mark_ini,
		std::string text,
		TiObj& var,
		std::function<std::string(Elisyum& el, TiObj& decl, TiObj& var)> func
	){

		string res, buf, buf2;
		int status = 0, ant=' ';
		for (int i=0; i<text.size(); i++){
			char c = text[i];

			if ( status == -1 ){
				if ( c == mark_ini )
					buf += c;
				else if ( c == '\\' )
					buf += c;
				else if ( c == '(' )
					buf += c;
				else if ( c == ')' )
					buf += c;
				else {
					buf += '\\';
					buf += c;
				}
				status = 0;
				
			// <br> aaa teste $class_name(tiobj) </br>
			} else if ( status == 0 ){
				if ( c == mark_ini ){
					status = 1;
				} else 
					res += c;
					
			// $class_name(tiobj) </br>
			} else if ( status == 1 ){
				if ( c == '(' )
					status = 2;
				else if ( isalnum(c) )
					buf += c;
				else {
					TiObj decl( Join("class='%s'").at(buf).ok );
					res += func(*this,decl,var);
					res += c;
					buf = buf2 = "";
					status = 0;
				}
			//(tiobj) </br>
			} else if ( status == 2 ){
				if ( c == ')' )
					status = 3;
				else
					buf2 += c;
			// ) </br>
			} else if ( status == 3 ){
				TiObj decl( Join("class='%s';%s").at(buf).at(buf2).ok );
				res += func(*this,decl,var);
				res += c;
				buf = buf2 = "";
				status = 0;
			}
			
			if ( c == '\\' )
				status = -1;
			
			ant = c;
		}
		return res;
	}
	
	
	TiObj* getObject(TiVar& var){
		if ( var.isObj() )
			return &var.atObj();
		TiObj* obj = new TiObj;
		obj->set("txt", var.atStr());
		return obj;
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
	
	/*Elisyum el;
	TiObj obj("class=Image;url=aaaa");
	cout << el.parseTmpl("<h1>@text(default=10;ini='<p>';end='</p>') </h1>",obj);*/
	
	
	return 0;
}
