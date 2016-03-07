#include<iostream>
#include<string.h>
#include<string>
#include<tiobj.hpp>
#include<tisys.hpp>
#include<stdio.h>
#include<stdlib.h>
#include<vector>
#include<dirent.h>

using namespace std;

// ticomplete cursor comando argumentos

//void ajustArgs(vector<string>& a)

bool isSubstr(std::string word, std::string text){
	if ( word.size() == 0 )
		return true;

	if (word.size() <=	 text.size() ){
		for (int i=0; i<word.size(); i++){
			if ( word[i] != text[i] )
				return false;
		}
		return true;
	} else {
		return false;
	}
}

class Option{
  public:
	std::string name;
	std::string real;

	Option(std::string name, std::string real){
		this->name = name;
		this->real = real;
	}
};



enum State {VERB, ACCUSATIVE, IN_PREPOSITION};


class AutoComplete {
  public:
	State state;
	TiObj param;
	TiObj syntax;
	int cur_pos;
	std::string cur;
	std::string method;
	std::string class_name;
	std::string o2cl_url;
	std::string prep;
	bool has_syntax;


	AutoComplete(int argc, char** argv, int cursor_pos){
		// Set the attributes
		this->state  = VERB;
		this->class_name = argv[0];
		this->cur_pos = cursor_pos;

		this->o2cl_url = "/o2cl";
		this->has_syntax = false;


		if ( this->cur_pos < argc )
			this->cur = argv[ this->cur_pos ];


		if ( argc > 1 ){
			bool is_state_set = false;
			this->method = argv[1];
			string noun, prep;

			if ( this->cur_pos >= 2 ){
				this->loadSyntax(this->class_name, this->method);
				this->state = ACCUSATIVE;
				State parse_state = ACCUSATIVE;
				for (int i=2; i<argc; i++){
					string word = argv[i];

					// Save the state of the cursor
					if ( i == this->cur_pos ){
						is_state_set = true;
						this->state = parse_state;
						if ( this->state == IN_PREPOSITION )
							this->prep = prep;
					}

					// Parse inside of Preposition
					if ( parse_state == IN_PREPOSITION ){
						noun += word;
						parse_state = ACCUSATIVE;

					// Parse accusative
					} else {
						if ( this->isPreposition(word) ){
							if ( noun != "" )
								param.set(prep,noun);
							prep = word;
							noun = "";
							parse_state = IN_PREPOSITION;
						} else {
							param.set("acc", word);
							prep = "";
						}
					}
				}

				if ( prep != "" ){
					if ( noun != "" ){
						param.set(prep,noun);
						if ( !is_state_set )
							this->state = ACCUSATIVE;
					} else {
						if ( !is_state_set ){
							this->state = IN_PREPOSITION;
							this->prep = prep;
						}
					}
//					is_state_set = true;
				}

			}



		}


	}












	/*void showMethods(){
		struct dirent *dp;
		DIR* dir = opendir(  );
		while ((dp = readdir(dir)) != NULL){
			if ( dp->d_name[0] == '.' )
				continue;
			if ( isSubstr(this->cur, dp->d_name) )
				cout << dp->d_name << " " << endl;
		}
		closedir(dir);
		
		//this->showFolder( Join("/class/%s/bin/%s").at(this->class_name).at("ma").ok );
	}*/
	
	
	void showSubMethods(){
		struct dirent *dp;
		DIR* dir = opendir( Join("%s/class/%s/bin/%s").at(this->o2cl_url).at(this->class_name).at(this->method).ok.c_str() );
		while ((dp = readdir(dir)) != NULL){
			if ( dp->d_name[0] == '.' )
				continue;
			if ( isSubstr(this->cur, dp->d_name) )
				cout << dp->d_name << ":" << endl;
		}
		closedir(dir);
	}


	void showPreposition();



	void showNoun( TiObj& script ){
		for (int i=0; i<script.size(); i++){
			TiObj& cmd = script.box[i];
			if ( cmd.classe == "ls" ){
				showFolder( this->cur, cmd.atStr("acc") );
			}
		}
	}



	bool isPreposition(std::string word);

	State getState();

	void loadSyntax(std::string classe, std::string method);

	void showAccusative();
	void showInPreposition();


	TiObj* getPrepositionObject(std::string prep);


	void showMethods(){
		string path, name;
		path = path_remove(this->method);
		name = path_last(this->method);


		Filesystem fs( Join("%s/class/%s/bin").at(this->o2cl_url).at(this->class_name).ok );
		fs.listdir( path );
		vector<Option> options;
		for (int i=0; i<fs.box.size(); i++){
			if ( isSubstr(name, fs.box[i].atStr("name")) ){
				if ( fs.box[i].is("Folder") )
					options.push_back( Option(fs.box[i].atStr("name"), fs.box[i].atStr("url")+"/") );
				else
					options.push_back( Option(fs.box[i].atStr("name"), fs.box[i].atStr("url")+" ") );
			}
		}


		if ( options.size() == 0 ){
			return;
		}



		if ( options.size() == 1 ){
			cout << options[0].real << endl;
		} else {
			int similarity = getSimilarity(options, name.size());
			if ( similarity == 0 ){
				for (int i=0; i<options.size(); i++){
					cout << options[i].name << endl;
				}
			} else {
				cout << path_add(path,options[0].name.substr(0,name.size()+similarity)) << endl;
			}
		}
	}








  private:
	int getSimilarity(vector<Option>& options, int ini){
		int min = 0xFFFF;
		string base = options[0].name;
		for (int i=1; i<options.size(); i++){
			int size = base.size();
			if ( options[i].name.size() < size )
				size = options[i].name.size();

			int cont = 0;
			for (int j=ini; j<size; j++){
				if ( base[j] != options[i].name[j] )
					break;
				cont+=1;
			}
			if ( cont < min )
				min = cont;
		}
		return min;
	}


	void showEnum(TiObj& obj){
		vector<Option> options;
		for (int i=0; i<obj.size(); i++){
			if ( isSubstr(this->cur, obj.box[i].atStr("name")) ){
				options.push_back( Option(obj.box[i].atStr("name"),"") );
			}
		}
		if ( options.size() == 1 ){
			cout << options[0].name << " " << endl;
		} else {
			for (int i=0; i<options.size(); i++){
				cout << options[i].name << endl;
			}
		}
	}











	void showFolder(std::string url, std::string type=""){
		Filesystem fs;

		string path;
		string name;

		if ( url == "/" ){
			path = "/";
		} else {
			path = path_remove(url);
			name = path_last(url);
		}

		fs.listdir(path);
		vector<Option> options;
		if ( type == "" ){
			for (int i=0; i<fs.box.size(); i++){
				if ( isSubstr(name, fs.box[i].atStr("name")) ){
					if ( fs.box[i].is("Folder") )
						options.push_back( Option(fs.box[i].atStr("name"), fs.box[i].atStr("url")+"/") );
					else
						options.push_back( Option(fs.box[i].atStr("name"), fs.box[i].atStr("url")+" ") );
				}
			}
		} else {
			for (int i=0; i<fs.box.size(); i++){
				if ( isSubstr(name, fs.box[i].atStr("name")) ){
					if ( fs.box[i].is("Folder") )
						options.push_back( Option(fs.box[i].atStr("name"), fs.box[i].atStr("url")+"/") );
					else if ( fs.box[i].is(type) )
						options.push_back( Option(fs.box[i].atStr("name"), fs.box[i].atStr("url")+" ") );
				}
			}
		}
		if ( name == "." || name == ".." ){
			options.push_back( Option("..", path_add(path,"../")) );
		}


		if ( options.size() == 0 ){
			return;
		}



		if ( options.size() == 1 ){
			cout << options[0].real << endl;
		} else {
			int similarity = getSimilarity(options, name.size());
			if ( similarity == 0 ){
				cout << ".." << endl;	// to avoid replace the parameter by similarity at option
				for (int i=0; i<options.size(); i++){
					cout << options[i].name << endl;
				}
			} else {
				cout << path_add(path,options[0].name.substr(0,name.size()+similarity)) << endl;
			}
		}
	}
	
	
	
	
	
	
};







/* Public ====*/



void AutoComplete::showAccusative(){
	this->showPreposition();
	if ( has_syntax && this->syntax.has("acc") ){
		this->showNoun( this->syntax.atObj("acc") );
	}
}


void AutoComplete::showInPreposition(){
	TiObj* script = this->getPrepositionObject( this->prep );
	if ( script ){
		this->showNoun( *script );
	}
}




/* Private ====*/

void AutoComplete::loadSyntax(std::string classe, std::string method){
	this->syntax.loadFile( Join("/o2cl/class/%s/syntax/%s.ti").at(classe).at(method).ok );
	if ( this->syntax.classe != "Error" ){
		this->has_syntax = true;
	}
}


TiObj* AutoComplete::getPrepositionObject(std::string prep){
	for (int i=0; i<this->syntax.size(); i++){
		TiObj& obj = this->syntax.box[i];
		if ( prep == obj.classe )
			return &obj;
	}
	return NULL;
}


void AutoComplete::showPreposition(){
	for (int i=0; i<this->syntax.size(); i++){
		TiObj& obj = this->syntax.box[i];
		if ( isSubstr(this->cur,obj.classe) )
			cout << obj.classe << endl;
	}
}



bool AutoComplete::isPreposition(std::string word){
	for (int i=0; i<this->syntax.size(); i++){
		TiObj& obj = this->syntax.box[i];
		if ( word == obj.classe )
			return true;
	}
	return false;
}

State AutoComplete::getState(){
	return this->state;
}



/*--------------------------------------------------------------------------------------*/







bool isFolder(string url){
	struct stat n_stat;
	if ( stat(url.c_str(), &n_stat) != -1 ){
		return (S_ISDIR(n_stat.st_mode));
	} else {
		return false;
	}
}









int main(int argc, char** argv){
	if ( argc < 3 )
		return 0;

	AutoComplete complete(argc-2,argv+2, atoi(argv[1]));
	int state = complete.getState();
//cout << state << endl;
		if ( state == VERB ){
		complete.showMethods();
	} else if ( state == ACCUSATIVE ){
		complete.showAccusative();
	} else if ( state == IN_PREPOSITION ){
		complete.showInPreposition();
	}
	return 0;
}


/*--------------------------------------------------------------------------------------*/
