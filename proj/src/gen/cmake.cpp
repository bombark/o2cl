#include <iostream>
#include <tiobj.hpp>
#include <stdlib.h>
#include <unistd.h>

using namespace std;

// cmake: install(DIRECTORIES ... USE_SOURCE_PERMISSIONS)
// cmake: install(PROGRAMS ...


/*void writeDepend(TiObj& obj){
	for (int i=0; i<obj.size(); i++){
		TiObj& node = obj.box[i];
		if ( node.is("Depend") ){
		}
	}
}*/

std::string getCodes(TiObj& obj){
	string res;
	if ( obj.has("src") )
		res = Join("\"%s\" ").at(obj.atStr("src")).ok;
	for (size_t i=0; i<obj.size(); i++){
		TiObj& node = obj.box[i];
		if ( node.is("Code") ){
			res += Join("\"%s\" ").at(node.atStr("name")).ok;
		}
	}
	return res;
}

std::string getDepends(TiObj& obj){
	string res;
	for (size_t i=0; i<obj.size(); i++){
		TiObj& node = obj.box[i];
		if ( node.is("Depends") ){
			res += Join("\"%s\" ").at(node.atStr("name")).ok;
		}
	}
	return res;
}


std::string getHeadersInstall(TiObj& obj){
	string res;
	for (size_t i=0; i<obj.size(); i++){
		TiObj& node = obj.box[i];
		if ( node.is("Header") ){
			res += Join("install(FILES \"%s\" DESTINATION include)\n").at(node.atStr("name")).ok;
		}
	}
	return res;
}




int main(int argc, char** argv){
	string nom = getenv("nom");
	chdir(nom.c_str());

	TiObj proj;
	proj.loadFile(".sysobj.ti");
	
	if ( proj.is("Error") ){
		cerr << proj;
		return 1;
	}
	
	
	// Build the Header
	cout << "cmake_minimum_required (VERSION 2.6)\n";
	cout << Join("project (\"%s\")\n").at(proj.atStr("name")).ok;
	cout << "set(CMAKE_BUILD_TYPE RELEASE)\n";
	if ( proj.has("compiler") ){
		if ( proj["compiler"].isObj() ){
			TiObj& compiler = proj["compiler"].atObj();
			string flags = compiler.atStr("flags");
			if ( !flags.empty() )
				cout << Join("set(CMAKE_CXX_FLAGS \"%s\")\n").at(flags).ok; 
		}
	}
	cout << "\n\n";


	for (int i=0; i<proj.size(); i++){
		TiObj& node = proj.box[i];
		if ( node.is("App") ){
			string name = node.atStr("name");
			string codes_files = getCodes(node);
			cout << Join("add_executable (%s %s)\n").at(name).at(codes_files).ok;


			string deps = getDepends(node);
			if ( deps.size() > 0 )
				cout << Join("target_link_libraries (%s %s)").at(name).at(deps).ok << endl;

			cout << Join("INSTALL(TARGETS %s DESTINATION bin)\n").at(name).ok;
		} else if ( node.is("Lib") ){
			string name = node.atStr("name");
			string codes_files = getCodes(node);
			string deps = getDepends(node);

			cout << Join("add_library (%s SHARED %s)\n").at(name).at(codes_files).ok;
			cout << Join("add_library (%s-static STATIC %s)\n").at(name).at(codes_files).ok;
			cout << Join("set_target_properties(%s-static PROPERTIES OUTPUT_NAME %s)\n").at(name).at(name).ok;
			if ( deps.size() > 0 )
				cout << Join("target_link_libraries (%s-statc %s)").at(name).at(deps).ok << endl;
			cout << Join("INSTALL(TARGETS \"%s\" DESTINATION lib)\n").at(name).ok;
			cout << Join("INSTALL(TARGETS \"%s-static\" DESTINATION lib)\n\n").at(name).ok;
			
			cout << getHeadersInstall(node) << endl;
		} else if ( node.is("File") ){
			string src = node.atStr("src");
			string  to = node.atStr("to","bin");
			cout << Join("INSTALL(FILES %s DESTINATION %s)\n").at(src).at(to).ok;
		}  else if ( node.is("Exec") ){
			string src = node.atStr("src");
			string  to = node.atStr("to","bin");
			cout << Join("INSTALL(PROGRAMS %s DESTINATION %s)\n").at(src).at(to).ok;
		}
	}
	return 0;
}
