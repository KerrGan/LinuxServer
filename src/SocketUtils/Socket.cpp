/*
 * Socket.cpp
 *
 *  Created on: 2016年3月24日
 *      Author: xiang
 */

#include "SocketUtils/Socket.h"

Socket::Socket() {
	// TODO Auto-generated constructor stub
	_fileDescriptor=0;
	_address={0};
	_port=0;
	_connectTimeOut=5;
}

Socket::Socket(int sock):Socket()
{
	_fileDescriptor=sock;
	_ip=SocketUtil::getSocketIPandPort(_fileDescriptor,_port);
}

Socket::~Socket() {
	// TODO Auto-generated destructor stub
	if(_fileDescriptor!=0)
		close();
}

int &Socket::getFileDescriptor()
{
	return _fileDescriptor;
}

sockaddr_in &Socket::getAddress()
{
	return _address;
}

int  Socket::getPort()
{
	return _port;
}

string  Socket::getIP()
{
	return _ip;
}

bool  Socket::connect(char* ip,int port)
{
	if ((_fileDescriptor = ::socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("create socket error: %s(errno: %d)\n", strerror(errno), errno);
	}

	memset(&_address, 0, sizeof(_address));
	_address.sin_family = AF_INET;
	_address.sin_port = htons(port);
	_address.sin_addr.s_addr=inet_addr(ip);

	_ip=ip;
	_port=port;

	unsigned long mode=1; //非阻塞模式
	fd_set set;
	ioctl(_fileDescriptor,FIONBIO,&mode);

	bool isConnect=false;
	timeval time={_connectTimeOut,0};
	int len=sizeof(time);
	int error=-1;

// in linux sol_sndtimeo is same as connect time out
//	setsockopt(_fileDescriptor,SOL_SOCKET,SO_SNDTIMEO,&time,len);

	if (::connect(_fileDescriptor, (struct sockaddr*) &_address,
			sizeof(_address)) == -1)
	{
		if(errno==EINPROGRESS)
		{
//			fprintf(stderr, "Connect time out.\n");
		}
		FD_ZERO(&set);
		FD_SET(_fileDescriptor,&set);
		int result=select(_fileDescriptor+1 , NULL, &set, NULL, &time);
		if (result > 0)
		{
			getsockopt(_fileDescriptor, SOL_SOCKET, SO_ERROR, &error,
					(socklen_t *) &len);
			if (error == 0)
				isConnect = true;
			else
				isConnect = false;
		} else if (result == 0) {
			isConnect = false;
			fprintf(stderr, "Connect time out.\n");
		} else if (result < 0) {
			isConnect=false;
			fprintf(stderr, "Connect occurs error.\n");
		}
	}
	else
	{
		isConnect=true;
	}


	if (!isConnect)
	{
//		printf("Connect error: %s(errno: %d)\n", strerror(errno), errno);
		shutdown(_fileDescriptor,2);
		fprintf(stderr, "Cannot connect the server!\n");
		return isConnect;
	}

	mode=0;
	ioctl(_fileDescriptor,FIONBIO,&mode);//已成功连接设置回阻塞模式

	cout<<"Connect success."<<endl;
	return isConnect;
}

int Socket::send(char* content,int length)
{
	int res=0;
	if ( (res=::send(_fileDescriptor, content, length, 0) )< 0)
	{
		printf("send msg error: %s(errno: %d)\n", strerror(errno), errno);
	}

	return res;
}

int Socket::receive(char* content,int length)
{
	int res;
	if (( res = ::recv(_fileDescriptor, content, length, 0)) == -1) {
		perror("recv error");
	}

	return res;
}

void Socket::close()
{
	::shutdown(_fileDescriptor,2);
	_fileDescriptor=0;
}

void Socket::setIP(char *ip)
{
	_ip=ip;
}

void Socket::setPort(int port)
{
	_port=port;
}

void Socket::setConnectTimeOut(int seconds)
{
	_connectTimeOut=seconds;
}

int Socket::getConnectTimeOut()
{
	return _connectTimeOut;
}

void Socket::setReciveTimeOut(int timeOut)
{
	timeval val;
	val.tv_sec=timeOut;
	val.tv_usec=0;
	if(setsockopt(_fileDescriptor,SOL_SOCKET,SO_RCVTIMEO,&val,sizeof(val))==-1)
	{
		printf("setReciveTimeOut error: %s(errno: %d)\n", strerror(errno), errno);
	}
}

void Socket::setSendTimeOut(int timeOut)
{
	timeval val;
	val.tv_sec=timeOut;
	val.tv_usec=0;
	if(setsockopt(_fileDescriptor,SOL_SOCKET,SO_SNDTIMEO,&val,sizeof(val))==-1)
	{
		printf("setSendTimeOut error: %s(errno: %d)\n", strerror(errno), errno);
	}
}

//timeval val;
//val.tv_sec=seconds;
//val.tv_usec=0;
//setsockopt(_fileDescriptor,SOL_SOCKET,SO_RCVTIMEO,&val,sizeof(val));
//
//timeval val;
//socklen_t len=sizeof(val);
//getsockopt(_fileDescriptor,SOL_SOCKET,SO_RCVTIMEO,&val,&len);
//return val.tv_sec;
