#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <string>
#include <arpa/inet.h>
#include <pthread.h>

#define BUF_SIZE  1023
#define MAXCONNECTIONS 5

int now_accept =0;
bool is_use[MAXCONNECTIONS] ={};
pthread_mutex_t mutex_lock;

using namespace std;

void* client_handler(void* data);

int main(int argc,char**argv){
  int listenSockFD;
  int clientSockFD;
  sockaddr_in server_addr,client_addr;
  listenSockFD = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
  if(listenSockFD<0){
    cout<<endl<<"socket create error"<<endl;
    return 0;
  }
  int on =1;
  if(setsockopt(listenSockFD,SOL_SOCKET,SO_REUSEADDR,(const char*)&on,sizeof(on))<0){
    cout<<endl<<"set option curLen =0 , Error"<<endl;
    return 0;
  }
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(30000);
  if(bind(listenSockFD,(struct sockaddr*)&server_addr,sizeof(server_addr))<0){
    cout<<endl<<"bind error"<<endl;
    return 0;
  }
  cout<<"server running. wait client..."<<endl;
  if(listen(listenSockFD,MAXCONNECTIONS)<0){
    cout<<endl<<"listen error"<<endl;
    return 0;
  }
  pthread_t p_thread[MAXCONNECTIONS];
  pthread_mutex_init(&mutex_lock, NULL);

  int clientAddrSize = sizeof(client_addr);
  while(true){
    pthread_mutex_lock(&mutex_lock);
    if(now_accept<MAXCONNECTIONS){
      clientSockFD = accept(listenSockFD,(struct sockaddr*) &client_addr,(socklen_t*) &clientAddrSize);
      if(clientSockFD<0){
        cout<<endl<<"accept error"<<endl;
        return -1;
      }
      cout<<endl<<"client accepted"<<endl;
      cout<<"address : "<<inet_ntoa(client_addr.sin_addr)<<endl;
      cout<<"port : "<<ntohs(client_addr.sin_port)<<endl;
      if(pthread_create(&p_thread[0], NULL, client_handler, (void *)&clientSockFD)<0){
        cout<<"THREAD ERROR"<<endl;
        return -1;
      }
      cout<<"create thread"<<endl;
    }
    else
      pthread_mutex_unlock(&mutex_lock);

  }
  close(listenSockFD);
  cout<<"server end"<<endl;
  return 0;
}

void* client_handler(void* data){
  int client = *((int*)data);
  int thread_index =0;
  while(is_use[thread_index]) 
    thread_index++;
  is_use[thread_index] = true;
  now_accept++;
  cout<<"Thread : "<<now_accept<<" running"<<endl;
  cout<<"Thread ID : "<<thread_index<<endl;
  pthread_mutex_unlock(&mutex_lock);

  char buf[BUF_SIZE] =" CLIENT ACCPETED!!";
  if(send(client,buf,BUF_SIZE,0)<0){
    cout<<client<<" SEND ERROR"<<endl;
    return NULL;
  }
  if(recv(client,buf,BUF_SIZE,0)<0){
    cout<<client<<" RECIEV ERROR"<<endl;
    return NULL;
  }
  pthread_mutex_lock(&mutex_lock);
  now_accept--;
  is_use[thread_index] = false;
  pthread_mutex_unlock(&mutex_lock);
}