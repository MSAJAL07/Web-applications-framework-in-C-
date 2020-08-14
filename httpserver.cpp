#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>


#include<iostream>
#include <thread> 
#include <unordered_map>
#include <map>
#include <sstream>
#include<algorithm>
using namespace std;


char * stringToChar(string str)
{
char *p=(char *)malloc((str.length()+1)*sizeof(char)); 
	p[str.length()]='\0';
    int i; 
    for (i = 0; i < str.length(); i++) { 
        p[i] = str[i]; 
    } 
return p;
}

typedef struct {
 string ext;
 string mediatype;
} extn;

extn extensions[] ={
 {"gif", "image/gif" },
 {"txt", "text/plain" },
 {"jpg", "image/jpg" },
 {"jpeg","image/jpeg"},
 {"png", "image/png" },
 {"ico", "image/ico" },
 {"zip", "image/zip" },
 {"gz",  "image/gz"  },
 {"js",  "text/javascript"},
 {"css",  "text/css"},
{"svg",  "image/svg+xml"},
 {"woff",  "font/woff"},
 {"woff2",  "font/woff2"},
 {"ttf",  "ttf/font"},
 {"tar", "image/tar" },
 {"htm", "text/html" },
 {"html","text/html" },
 {"php", "text/html" },
 {"pdf","application/pdf"},
 {"json","application/json"},
 {"zip","application/octet-stream"},
 {"rar","application/octet-stream"},
 {"0","0"} };

class ServiceResolver;
class HttpResponse;
class HttpRequest;
typedef struct {
ServiceResolver * serviceResolver=NULL;
int socket;
}IDTK;





//**************************************************************************************************************

class Service
{
public:
virtual void processGetService(HttpRequest * req,HttpResponse * res)=0;
virtual void processPostService(HttpRequest * req,HttpResponse * res)=0;
};

class GetService : public Service
{
public:
void processPostService(HttpRequest * req, HttpResponse * res)
{
cout<<"Post request not allowed\n";
}
};

class PostService : public Service
{
public:
void processGetService(HttpRequest * req, HttpResponse * res)
{
cout<<"Get request not allowed\n";
}
};

class ServiceResolver
{
public:
virtual Service * getService(string str)=0;
};



//**************************************************************************************************************





class HttpRequest
{
private:
string url;
string type;
unordered_map<string,string> mp;
map <string,string> cookies;


public:
HttpRequest(string req)
{
//cout<<"\n\ndkjdsfhksdhfk-------\n\n"<<req<<'\n';
stringstream s(req);
string tmp;
getline(s,tmp,'\n');
cout<<"***"<<tmp<<'\n';
stringstream ss(tmp);
getline(ss,type,' ');
getline(ss,tmp,' ');
cout<<"type"<<"="<<type<<'\n';
if(tmp == "/") url ="/index.html";
else
{
size_t found=tmp.find('?');
if (found != string::npos)
{
url = tmp.substr(0,found);
tmp = tmp.substr(found+1);
//cout<<"tmp="<<tmp<<'\n';
stringstream sss(tmp);
string key,val="";
while( getline(sss,key,'='))
{
getline(sss,val,'&');
mp.insert(make_pair(key,val));
cout<<key<<"== "<<val<<'\n';
}
}
else
url = tmp;
}
cout<<"url="<<url<<'\n';
string cookie;
while(s)
{
getline(s,tmp,'\n');
if(tmp.size()>1)
{
cookie=tmp;
}
}
cout<<cookie<<'\n';
stringstream sss(cookie);
getline(sss,tmp,' ');
if(tmp == "Cookie:")
{
string key,val="";
while( getline(sss,key,'='))
{
getline(sss,val,' ');
if(val[val.size()-1] == ';') val = val.substr(0,val.size()-1);
cookies.insert(make_pair(key,val));
cout<<key<<"== "<<val<<'\n';
}
}
//constructor ends
}


string getUrl()
{
return url;
}

string getType()
{
return type;
}

string getPerameter(string key)
{
 unordered_map<string,string>::iterator itr = mp.find(key);
 if(itr != mp.end()) return itr->second;
 else return "";
}


string getCookie(string key)
{
 map<string,string>::iterator itr = cookies.find(key);
 if(itr != cookies.end()) return itr->second;
 else return "";
}


//class ends
};


class HttpResponse
{
int socket;
IDTK *idtk;
HttpRequest * req;
int status;
string header;
string contentType;
string cookie;
string message;
void createHeader(int status, string message, string contentType, string cookie)
{
this->header = "HTTP/1.1 "+to_string(status)+" "+message+"\r\nAccess-Control-Allow-Headers: Origin, X-Requested-With, Content-Type, Accept\r\nAccess-Control-Allow-Origin: http://localhost:4200 \r\n"+contentType+"\r\n"+cookie+"\r\n\r\n";
}
void createHeader()
{
header = "HTTP/1.1 "+to_string(status)+" "+message+"\r\nAccess-Control-Allow-Headers: Origin, X-Requested-With, Content-Type, Accept\r\nAccess-Control-Allow-Origin: http://localhost:4200 \r\n"+contentType+"\r\n"+cookie+"\r\n\r\n";
}


string getContentType(string url)
{
reverse(url.begin(), url.end());
int i =url.find('.');
if (i != string::npos)
{
string ext=url.substr(0,i);
reverse(ext.begin(), ext.end());
cout<<"********************************   "<<ext<<'\n'; 
for (i = 0; extensions[i].ext !="0"; i++)
{
if(ext == extensions[i].ext)
{
return extensions[i].mediatype;
}
}
}
return "text/plain";
//function ends
}




void sendFile(string url)
{
char * file_name=stringToChar("public"+url);
//cout<<"URL --- "<<file_name<<'\n';
//cout<<"fileName --- "<<rq.getUrl()<<'\n';
cout<<"fileName --- "<<file_name<<'\n';
FILE *f = fopen(file_name,"rb");
if(f==NULL)
{
cout<<"File not found Error\n";
char error[1024];
sprintf(error,"<html><head><title>404 Not Found</head></title><body><p>404 Not Found: The requested resource could not be found!</p></body></html>");
sendHeader(404);
send(socket,error,strlen(error), 0);
}
else
{
cout<<"File  found \n";
int byteCount;
char buffer[1024]={'0'};
setContentType(getContentType(url));
sendHeader(200);
//char header[1024];
//sprintf(header,"HTTP/1.1 200 OK\r\nContent-Type:%s\r\n\r\n",stringToChar());
//send(socket,header,strlen(header), 0);
cout<<"File  found sdadad \n";
 while( (byteCount= fread(buffer, 1,1024, f))>0 ){
        send(socket,buffer,byteCount, 0);
    }

}
cout<<"File  found ddd\n";
close(socket);
//function ends
}



public :
HttpResponse(IDTK * idtk ,HttpRequest * req)
{
this->socket = idtk->socket;
this->status = 0;
this->idtk = idtk;
this-> req = req;
}

void setStatus(int status)
{
this->status = status;
if(status == 200) message = "Ok";
if(status == 404) message = "Not Found";
}
void setContentType(string str)
{
this->contentType="Content-Type : " +str;
}
void setCookie(string str)
{
this->cookie = this->cookie+"Set-Cookie: "+str+"\r\n";
}
void sendHeader(int status)
{
setStatus(status);
createHeader();
cout<<"\n\n\n"<<this->header<<"\n\n\n";
char * head = stringToChar(this->header);
send(this->socket,head,strlen(head),0);
}

void process(string url)
{
cout<<"chal1\n";
string ext;
//cout<<"chal2\n";
//string url = hr.getUrl();
//cout<<"chal\n";

reverse(url.begin(), url.end());

int i =url.find('.');
if (i != string::npos)
{
ext=url.substr(0,i);
reverse(ext.begin(), ext.end());
}
reverse(url.begin(), url.end());
cout<<"chal "<<ext<<" "<<url<<'\n';
if( ext=="ss" && idtk->serviceResolver != NULL)
{
cout<<"chal11\n";
ServiceResolver * serviceResolver = idtk->serviceResolver;
Service * service;
service =serviceResolver->getService(url);
//cout<<"chal111\n";
if (service != NULL)
{
if( req->getType() == "GET" ) service->processGetService(req,this);
else service->processPostService(req,this);
}
else
sendFile(url);
}
else
sendFile(url);
// process ends
}





void out(string str)
{
char * content = stringToChar(str);
send(this->socket,content,strlen(content),0);
}
void flush()
{
close(this->socket);
}

//class ends
};

class RequestProcessor
{
char buffer[30000]={0};
int byteCount=1;
public:
//operator()
void foo(IDTK * idtk) 
{
cout<<"andar aya\n";
byteCount=recv(idtk->socket,buffer,sizeof(buffer),0);
string s=buffer;
cout<<s<<'\n';
if(s.size() ==0) 
{
cout<<"aya\n";
close(idtk->socket);
return;
} 
HttpRequest *hr= new HttpRequest(s);
HttpResponse *res = new HttpResponse(idtk,hr);
res->process(hr->getUrl());

/*
cout<<"chal1\n";
string ext;
cout<<"chal2\n";
string url = hr.getUrl();
cout<<"chal\n";

reverse(url.begin(), url.end());

int i =url.find('.');
if (i != string::npos)
{
ext=url.substr(0,i);
reverse(ext.begin(), ext.end());
}
reverse(url.begin(), url.end());
cout<<"chal "<<ext<<" "<<url<<'\n';
if( ext=="ss" && idtk->serviceResolver != NULL)
{
cout<<"chal11\n";
ServiceResolver * serviceResolver = idtk->serviceResolver;
Service * service;
service =serviceResolver->getService(url);
//cout<<"chal111\n";
if( hr.getType() == "GET" ) service->processGetService(hr,hres);
else service->processPostService(hr,hres);
}
else

hres.sendFile(url);
*/
//operator() ends
// foo ends
}


//class ends
};




class HttpServer
{
private:
int server_dis,new_socket;
struct sockaddr_in address;
int addrlen;
ServiceResolver * serviceResolver=NULL;

public:
 

void setServiceResolver(ServiceResolver * sr)
{
this->serviceResolver = sr;
}

void start(string ip ,int port)
{
this->addrlen = sizeof(address);
if((server_dis = socket(AF_INET,SOCK_STREAM,0)) == 0)
{
cout<<"Unable to create a socket\n";
return;
}

address.sin_family = AF_INET;
address.sin_addr.s_addr = htons(INADDR_ANY);
address.sin_port = htons(port);
memset(address.sin_zero, '\0', sizeof address.sin_zero);
if (bind(server_dis, (struct sockaddr *)&address, sizeof(address))<0)
    {
        perror("In bind");
       return;
    }
if (listen(server_dis, 10) < 0)
    {
        cout<<"In listen\n";
        return;
    }
cout<<"myserver is running on port:"<<port<<'\n';
while(1)
{
        if ((new_socket = accept(server_dis, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
        {
            cout<<"Unable to connect to client\n";
        }
	else
	{
cout<<"*******************************************ayaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n";
		IDTK idtk;
		idtk.socket=new_socket;
		idtk.serviceResolver=this->serviceResolver;
		//new thread(RequestProcessor(),&idtk);
		RequestProcessor *r =new RequestProcessor();
		r->foo(&idtk);
	}


}
//start ends
}
//class ends
};
//**************************************************************************************888888888***************************************************************************************
//**************************************************************************************USER CODE****************************************************************************************
class Aa :public Service
{

public :
void processGetService(HttpRequest *req,HttpResponse *res)
{
string user = req->getPerameter("user");
string pass = req->getPerameter("pass");
cout<<"user "<<user<<" PASS "<<pass<<'\n';
if( user == "sajal" && pass == "mishra" )
{
//res.setContentType("text/html");
//res.init();
res->setCookie("yummy_cookie=choco  Max-Age=1000");
res->process("/test1.ss");
}
else
{
res->setContentType("text/html");
//res.init();
res->setCookie("yummy_cookies=choco0000000000  Max-Age=1000");
res->sendHeader(200);
string link ="/register.html";
res->out("<html><head><title>Can not login</title></head><body><center><a href='"+link+"' >Ragister</a></center></body></html>");
res->flush();
}
}

void processPostService(HttpRequest *req,HttpResponse *res)
{
cout<<"post chali\n";
}

};

class Bb : public GetService
{


void processGetService(HttpRequest *req,HttpResponse *res)
{
cout<<"Cookie : "<<req->getCookie("yummy_cookies")<<'\n';
res->process("/index.html");
}
};

class ABC : public ServiceResolver
{
public:
Service * getService(string str)
{
cout<<"getService "<<str<<'\n';
if (str == "/test.ss")
{
Service * s= new Aa();
return s;
}
if(str == "/test1.ss")
{
Service * s= new Bb();
return s;
} 
return NULL;
}


};




int main(int argc, char const *argv[])
{
HttpServer hs;
hs.setServiceResolver(new ABC());
hs.start("localhost",8000);
return 0;
}
