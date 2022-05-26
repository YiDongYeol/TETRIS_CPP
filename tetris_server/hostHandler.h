#pragma once
#define _CRT_SECURE_NO_WARNINGS
#define TET_PORT 9500
#include <iostream>
#include <string>
#include <thread>
#include <WinSock2.h>
#pragma comment(lib, "wsock32.lib")
using namespace std;

typedef struct {
	int port;
	bool used;
}port_list;
typedef struct {
	string identify;
	SOCKET AccSock;
	SOCKADDR_IN ClieAdd;
	thread _listen;
	bool fail;
	bool ready;
} member_list;

class host {
private:
	static int last_host_number;
	static int host_count;
	int index;
	int host_number;
	int port;
	int member_count;
	int grade;
	bool is_start;
	bool is_closed;
	member_list m_list[3];
	void threading(int m_c);
	void listen_thread_func(int ind);
public:
	host(int index, string identify, int port) :index(index), port(port), host_number(last_host_number), member_count(1), is_start(false), is_closed(false)
	{
		for (int i = 0; i < 3; i++) {
			memset(&(m_list[i]), 0, sizeof(member_list));
			m_list[i].identify = string("\0");
			m_list[i].fail = false;
			m_list[i].ready = false;
		}
		last_host_number++;
		host_count++;
		m_list[0].identify = identify;
		threading(0);
	}
	~host()
	{
		host_count--;
	}
	bool getIsStart() { return is_start; }
	bool getIsClosed() { return is_closed; }
	int getPort() { return port; }
	static int getLastHostNumber() { return last_host_number; }
	int getHostNumber() { return host_number; }
	int getMemberCount() { return member_count; }
	bool getMemberReady(int index){ return m_list[index].ready; }
	static int getHostCount() { return host_count; }
	void addMember(string identify);
	string getMemberID(int index) { return m_list[index].identify; }
	int check_member_id(string nid);
};

void startServe();