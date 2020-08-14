#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>


#include<iostream>
#include <thread> 
#include <unordered_map>
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
 {"tar", "image/tar" },
 {"htm", "text/html" },
 {"html","text/html" },
 {"php", "text/html" },
 {"pdf","application/pdf"},
 {"zip","application/octet-stream"},
 {"rar","application/octet-stream"},
 {"0","0"} };

class ServiceResolver;

typedef struct {
ServiceResolver * serviceResolver=NULL;
int socket;
}IDTK;







class HttpRequest
{
private:
string url;
string type;
unordered_map<string,string> mp;


public:
HttpRequest(string req)
{
//cout<<"\n\ndkjdsfhksdhfk-------\n\n"<<req<<'\n';
stringstream s(req);
string tmp;
getline(s,tmp,'\n');
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

//class ends
};


class HttpResponse
{
public :
int socket;
HttpResponse(int socket)
{
this->socket = socket;
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
char header[1024];
sprintf(header,"HTTP/1.1 404 Not Found\r\nServer : Web Server in C\r\n\r\n<html><head><title>404 Not Found</head></title><body><p>404 Not Found: The requested resource could not be found!</p></body></html>");
send(socket,header,strlen(header), 0);
}
else
{
cout<<"File  found \n";
int byteCount;
char buffer[1024]={'0'};
char header[1024];
sprintf(header,"HTTP/1.1 200 OK\r\nContent-Type:%s\r\n\r\n",stringToChar(getContentType(url)));
send(socket,header,strlen(header), 0);
cout<<"File  found sdadad \n";
 while( (byteCount= fread(buffer, 1,1024, f))>0 ){
        send(socket,buffer,byteCount, 0);
    }

}
cout<<"File  found ddd\n";
close(socket);
//constructor ends
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

//class ends
};
//**************************************************************************************************************

class Service
{
public:
virtual void processGetService(HttpRequest req,HttpResponse res)=0;
virtual void processPostService(HttpRequest req,HttpResponse res)=0;
};

class GetService : public Service
{
public:
void processPostService(HttpRequest req, HttpResponse res)
{
cout<<"Post request not allowed\n";
}
};

class PostService : public Service
{
public:
void processGetService(HttpRequest req, HttpResponse res)
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
HttpRequest hr(s);
HttpResponse hres(idtk->socket);
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
cout<<"chal111\n";
if( hr.getType() == "GET" ) service->processGetService(hr,hres);
else service->processPostService(hr,hres);
}
else

hres.sendFile(url);
//operator() ends
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
//************************************************USER CODE********************************************************
class Aa :public Service
{

public :
void processGetService(HttpRequest req,HttpResponse res)
{
string user = req.getPerameter("user");
string pass = req.getPerameter("pass");
cout<<"user "<<user<<" PASS "<<pass<<'\n';
if( user == "sajal" && pass == "mishra" )
{
//res.setContentType("text/html");
//res.init();
res.sendFile("/index.html");
}
else
{
//res.setContentType("text/html");
//res.init();
res.sendFile("/register.html");
}
}

void processPostService(HttpRequest req,HttpResponse res)
{
cout<<"post chali\n";
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
else NULL;
}


};




int main(int argc, char const *argv[])
{
HttpServer hs;
hs.setServiceResolver(new ABC());
hs.start("localhost",8080);
return 0;
}
