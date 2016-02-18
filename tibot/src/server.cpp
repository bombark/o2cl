

/*====================================- HEADER -========================================*/

#include <microhttpd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <unistd.h>
#include <sys/stat.h>
#include <tiobj.hpp>

using namespace std;


#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"

/*--------------------------------------------------------------------------------------*/





/*=====================================- Server -=======================================*/

class EmbeddedServer {
  private:
	struct MHD_Daemon * daemon;
	std::string root;

  public:
	EmbeddedServer(int port=4242);
	~EmbeddedServer();

	static int ahc_echo(
		void * cls,
		struct MHD_Connection* connection,
		const char*  url,
		const char*  method,
		const char*  version,
		const char*  upload_data,
		size_t *     upload_data_size,
		void **      ptr
	);
};

/*--------------------------------------------------------------------------------------*/


/*=====================================- Context -======================================*/

class Context {
  public:
	struct MHD_PostProcessor* pp;
	std::string get;
	std::string post;
};

/*--------------------------------------------------------------------------------------*/


/*===================================- Connection -=====================================*/


class Connection {
  public:
	std::string url;
	struct MHD_Connection* server_conn;
	struct MHD_Response* response;
	int response_code;


	clock_t time_ini;


	Connection(struct MHD_Connection* server_conn, std::string url);

	void sendFile(std::string _url);
	void sendText(std::string text);

	void createCookie();
	void redirect(std::string url);

	void error(int num, std::string msg);
	int  finish();
};

/*--------------------------------------------------------------------------------------*/







/*================================- Code Connection -===================================*/

Connection::Connection(struct MHD_Connection* server_conn, std::string url){
	time_ini = clock();
	this->url = url;
	this->server_conn = server_conn;
	this->response_code = MHD_HTTP_NOT_FOUND;
}


void Connection::sendFile(std::string _url){
	string url = "/etc/" + _url;
	struct stat st;
	if(stat(url.c_str(), &st) != 0) {
		this->error(MHD_HTTP_NOT_FOUND,"Page not Found");
	} else {
		this->response_code = MHD_HTTP_OK;
		size_t size = st.st_size;
		FILE* fd = fopen(url.c_str(), "r");
		this->response = MHD_create_response_from_fd(size,fileno(fd));
	}
}


void Connection::sendText(std::string text){
	this->response_code = MHD_HTTP_OK;
	this->response = MHD_create_response_from_buffer(text.size(),(void*) text.c_str(), MHD_RESPMEM_MUST_COPY);
}


void Connection::createCookie(){
	char cstr[256];
	snprintf (cstr,sizeof(cstr),"id=aaaaa");
	int error = MHD_add_response_header(this->response,MHD_HTTP_HEADER_SET_COOKIE,cstr);
	if ( error == MHD_NO ){
		fprintf (stderr, "Failed to set session cookie header!\n");
	}
}


void Connection::redirect(std::string url){
	this->response_code = MHD_HTTP_FOUND;
	if (this->response == NULL){
		string text = "";
		this->response = MHD_create_response_from_buffer(text.size(),(void*) text.c_str(), MHD_RESPMEM_MUST_COPY);
	}
	MHD_add_response_header (this->response, "Location", url.c_str());
}


void Connection::error(int code, std::string msg){
	this->response_code = code;
	this->response = MHD_create_response_from_buffer(msg.size(),(void*) msg.c_str(), MHD_RESPMEM_MUST_COPY);
}


int Connection::finish(){
	if ( this->response == NULL )
		this->error(MHD_HTTP_NOT_FOUND, "Page not found");

	clock_t time_end = clock();
	double elapsed_secs = double(time_end - time_ini) / CLOCKS_PER_SEC;

	if ( this->response_code == MHD_HTTP_OK )
		printf("[%d:(%.3f)]: %s%s%s\n", this->response_code, elapsed_secs, KGRN, url.c_str(), KNRM);
	else
		printf("[%d:(%.3f)]: %s%s%s\n", this->response_code, elapsed_secs, KRED, url.c_str(), KNRM);

	int ret = MHD_queue_response(this->server_conn,this->response_code,this->response);
	MHD_destroy_response(this->response);
	return ret;
}

/*--------------------------------------------------------------------------------------*/



/*===========================- Callback - Get and Post -================================*/

int web_config_get (void *cls, enum MHD_ValueKind kind, const char *key, const char *value){
	std::string& get = *((std::string*) cls);
	get += string("  ") + key + "=" + value + "<br>\n";
	return MHD_YES;
}


int web_config_post (
	void *cls, enum MHD_ValueKind kind, const char *key, const char *filename,
	const char *content_type, const char *transfer_encoding,
	const char *data, uint64_t off, size_t size
){
	if ( filename ){
		printf("filename %s; type %s\n",filename,content_type);
	} else {
		printf("%s\n",data);
	}
	return MHD_YES;
}

/*--------------------------------------------------------------------------------------*/





/*===================================- Code Server -====================================*/

EmbeddedServer::EmbeddedServer(int port){
	this->daemon = MHD_start_daemon(
		MHD_USE_THREAD_PER_CONNECTION,
		port,
		NULL,
		NULL,
		&EmbeddedServer::ahc_echo,
		this,
		MHD_OPTION_END
	);
	printf("Server work in localhost:%d\n",port);
}

EmbeddedServer::~EmbeddedServer(){
	if (this->daemon)
		MHD_stop_daemon(this->daemon);
}

int EmbeddedServer::ahc_echo(
	void* _server,
	struct MHD_Connection* connection,
	const char* url,
	const char* method,
	const char* version,
	const char* upload_data,
	size_t*     _upload_data_size,
	void**      con_cls
) {


	Context* context = (Context*) *con_cls;
	size_t upload_data_size = *_upload_data_size;
	if (context == NULL){
		context = new Context();
		if ( strcmp(method,"POST") == 0 )
			context->pp = MHD_create_post_processor(connection, 1024, web_config_post, (void*)&context->get );
		*con_cls = context;
		return MHD_YES;
	}
	if (upload_data_size > 0 ){
		MHD_post_process(context->pp, upload_data, upload_data_size);
		*_upload_data_size = 0;
		return MHD_YES;
	} else {
		MHD_destroy_post_processor(context->pp);
	}
/*
MHD_RESPONSE_HEADER_KIND 	Response header
MHD_HEADER_KIND 	HTTP header.
MHD_COOKIE_KIND 	Cookies. Note that the original HTTP header containing the cookie(s) will still be available and intact.
MHD_POSTDATA_KIND 	POST data. This is available only if a content encoding supported by MHD is used (currently only URL encoding), and only if the posted content fits within the available memory pool. Note that in that case, the upload data given to the MHD_AccessHandlerCallback will be empty (since it has already been processed).
MHD_GET_ARGUMENT_KIND 	GET (URI) arguments.
MHD_FOOTER_KIND */

	MHD_get_connection_values (connection, MHD_GET_ARGUMENT_KIND, web_config_get, (void*)&context->get);
	EmbeddedServer& server = *((EmbeddedServer*)_server);
	Connection conn(connection, url);


	string text;
	text = "<html><body>Ok!<br>";
	text += context->get;
	text += context->post;
	text += "</body></html>"; 
	conn.sendText( text );

	delete context;
	return conn.finish();
}

/*--------------------------------------------------------------------------------------*/






/*=====================================- MAIN -=========================================*/


int main(int argc, char ** argv) {
	TiObj params;
	params.loadText( getenv("params") );
	
	string nom = params.atStr("nom");
	chdir( nom.c_str() );


	EmbeddedServer server;
	while(1)
		sleep(100);
	return 0;
}

/*--------------------------------------------------------------------------------------*/
