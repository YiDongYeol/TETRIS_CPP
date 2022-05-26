#include "hostHandler.h"

host* h_list[50];
port_list p_list[50];

void host::addMember(string identify) {
	for (int i = 0; i < 3; i++) {
		if (m_list[i].AccSock == 0) {
			m_list[i].identify = identify;
			threading(i);
			member_count++;
			return;
		}
	}
}
void host::listen_thread_func(int ind) {
	if (ind != 0) {
		char msg[50];
		sprintf(msg, "NEW%d%-10s", ind, &m_list[ind].identify[0]);
		for (int i = 0; i < 3; i++) {
			if (i == ind) continue;
			if (m_list[i].AccSock == 0) continue;
			send(m_list[i].AccSock, msg, sizeof(msg), 0);
		}
	}
	while (true) {
		char recv_arr[256];
		if (recv(m_list[ind].AccSock, recv_arr, 256, 0) == -1) {
			if (ind == 0 && !is_start)
				is_closed = true;
			if (!m_list[ind].fail) {
				for (int i = 0; i < 3; i++) {
					if (m_list[i].AccSock == 0) continue;
					if (i == ind) continue;
					string end_msg = "\0";
					end_msg += "EXIT";
					end_msg += (ind + '0');
					if (is_start) end_msg += (grade + '0');
					send(m_list[i].AccSock, &(end_msg[0]), 256, 0);
				}
				if (is_start) grade--;
				if (grade == 0) {
					for (int i = 0; i < 3; i++) {
						if (m_list[i].AccSock == 0) continue;
						if (i == ind) continue;
						send(m_list[i].AccSock, "GEND", 5, 0);
					}
				}
			}
			closesocket(m_list[ind].AccSock);
			memset(&(m_list[ind]), 0, sizeof(member_list));
			m_list[ind].identify = string("\0");
			member_count--;
			if (member_count == 0) {
				for (int i = 0; i < 50; i++) {
					if (p_list[i].port == port) {
						p_list[i].used = false;
						break;
					}
				}
				for (int i = 0; i < 50; i++) {
					if (h_list[i] == this) {
						h_list[i] = 0;
						break;
					}
				}
				delete this;
			}
			break;
		}
		string recv_str(recv_arr);
		string part_str = recv_str.substr(1, recv_str.length() - 1);
		//시작 전
		if (recv_str[0] == '0') {
			closesocket(m_list[ind].AccSock); 
			memset(&(m_list[ind]), 0, sizeof(member_list));
			m_list[ind].identify = string("\0");
			member_count--;
			if (member_count == 0) {
				for (int i = 0; i < 50; i++) {
					if (p_list[i].port == port) {
						p_list[i].used = false;
						break;
					}
				}
				for (int i = 0; i < 50; i++) {
					if (h_list[i] == this) {
						h_list[i] = 0;
						break;
					}
				}
				delete this;
			}
			break;
		}
		else if (recv_str[0] == '1') {
			if (part_str == "READY") {
				m_list[ind].ready = true;
				char rd_tmp[256];
				sprintf(rd_tmp, "READY%d", ind);
				for (int i = 0; i < 3; i++) {
					if (m_list[i].AccSock == 0) continue;
					send(m_list[i].AccSock, rd_tmp, 256, 0);
				}
			}
			else if (part_str == "CANCLE") {
				m_list[ind].ready = false;
				char rd_tmp[256];
				sprintf(rd_tmp, "CANCLE%d", ind);
				for (int i = 0; i < 3; i++) {
					if (m_list[i].AccSock == 0) continue;
					send(m_list[i].AccSock, rd_tmp, 256, 0);
				}
			}
			else if (part_str == "START") {
				for (int i = 0; i < 3; i++) {
					if (m_list[i].AccSock == 0) continue;
					send(m_list[i].AccSock, "START", 6, 0);
				}
				is_start = true;
				grade = member_count;
			}
		}
		//시작
		else if (recv_str[0] == '2') {
			if (part_str == "OVER") {
				break;
			}
			else if (part_str == "DIE") {
				m_list[ind].fail = true;
				string gov_str = "\0";
				gov_str += (ind + '0');
				gov_str += "DIE"; 
				gov_str += (grade + '0');
				grade--;
				for (int i = 0; i < 3; i++) {
					if (m_list[i].AccSock == 0) continue;
					send(m_list[i].AccSock, &(gov_str[0]), 256, 0);
				}
				if (grade == 0) {
					for (int i = 0; i < 3; i++) {
						if (m_list[i].AccSock == 0) continue;
						send(m_list[i].AccSock, "GEND", 5, 0);
					}
					for (int i = 0; i < 50; i++) {
						if (p_list[i].port == port) {
							p_list[i].used = false;
							break;
						}
					}
					for (int i = 0; i < 50; i++) {
						if (h_list[i] == this) {
							h_list[i] = 0;
							break;
						}
					}
					delete this;
					return;
				}
			}
			else {
				string blk_str = "\0";
				blk_str += (ind + '0');
				blk_str += part_str;
				for (int i = 0; i < 3; i++) {
					if (m_list[i].AccSock == 0) continue;
					send(m_list[i].AccSock, &(blk_str[0]), 256, 0);
				}
			}
		}
	}
}
void host::threading(int m_c) {
	SOCKET ConSock;
	SOCKADDR_IN ServAdd;
	int len = sizeof(SOCKADDR_IN);
	ConSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (ConSock == -1) {
		cout << "생성 실패" << endl;
		return;
	}
	ServAdd.sin_family = AF_INET;
	ServAdd.sin_addr.s_addr = htonl(INADDR_ANY);
	ServAdd.sin_port = htons(port);
	if (bind(ConSock, (SOCKADDR*)&ServAdd, sizeof(ServAdd)) == -1) {
		cout << "바인딩 실패" << endl;
		return;
	}
	if (listen(ConSock, 1024) == -1) {
		cout << "리슨 실패" << endl;
		return;
	}
	while (true) {
		m_list[m_c].AccSock = accept(ConSock, (SOCKADDR*)&(m_list[m_c].ClieAdd), &len);
		char recv_str[256];
		recv(m_list[m_c].AccSock, recv_str, 256, 0);
		string identify(recv_str);
		identify = identify.substr(0, identify.find(" "));
		if (identify == m_list[m_c].identify) {
			send(m_list[m_c].AccSock, "SUCCESS", 8, 0);
			break;
		}
		else {
			send(m_list[m_c].AccSock, "FAIL", 5, 0);
			closesocket(m_list[m_c].AccSock);
		}
	}

	m_list[m_c]._listen = thread(&host::listen_thread_func, this, m_c);
	m_list[m_c]._listen.detach();
	closesocket(ConSock);
}
int host::check_member_id(string nid) {
	for (int i = 0; i < 3; i++) {
		if (m_list[i].AccSock == 0) continue;
		if (m_list[i].identify == nid)
			return -1;
	}
	return 0;
}
void startServe() {
	for (int i = 0; i < 50; i++)
		h_list[i] = 0;
	for (int i = 0; i < 50; i++) {
		p_list[i].port = TET_PORT + 1 + i;
		p_list[i].used = false;
	}
	SOCKET ConSock;
	SOCKADDR_IN ServAdd;

	int len = sizeof(SOCKADDR_IN);
	ConSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (ConSock == -1) {
		cout << "생성 실패" << endl;
		return;
	}
	ServAdd.sin_family = AF_INET;
	ServAdd.sin_addr.s_addr = htonl(INADDR_ANY);
	ServAdd.sin_port = htons(TET_PORT);
	if (bind(ConSock, (SOCKADDR*)&ServAdd, sizeof(ServAdd)) == -1) {
		cout << "바인딩 실패" << endl;
		return;
	}
	if (listen(ConSock, 1024) == -1) {
		cout << "리슨 실패" << endl;
		return;
	}

	while (1) {
		SOCKADDR_IN ClieAdd;
		SOCKET AccSock = accept(ConSock, (SOCKADDR*)&(ClieAdd), &len);
		char msg[256];
		if (AccSock == -1) {
			cout << "받기 실패" << endl;
			continue;
		}
		recv(AccSock, msg, 256, 0);
		string str_msg(msg);


		if (str_msg[0] == '0')			// 호스트 리스트 요청 
		{
			if (host::getHostCount() == 0)
				send(AccSock, "0", 2, 0);
			else {
				string list_msg = "\0";
				string host_c_msg = "\0";
				char tmp[256];
				int host_c = 0;
				for (int i = 0; i < 50; i++) {
					if (h_list[i] == 0) continue;
					sprintf(tmp, "%06d", h_list[i]->getHostNumber());
					list_msg += tmp;
					sprintf(tmp, "%-10s", &(h_list[i]->getMemberID(0)[0]));
					list_msg += tmp;
					list_msg += to_string(h_list[i]->getMemberCount());
					if (h_list[i]->getIsStart()) { list_msg += 'T'; }
					else { list_msg += 'F'; }
					host_c++;
				}
				sprintf(tmp, "%02d", host_c);
				host_c_msg += tmp;
				host_c_msg += list_msg;
				send(AccSock, &(host_c_msg[0]), 256, 0);
			}
		}
		else if (str_msg[0] == '1')			// 호스트 생성
		{
			int empty_port = 0;
			string identify = str_msg.substr(1, 10);
			identify = identify.substr(0, identify.find(" "));
			for (int i = 0; i < 50; i++) {
				if (p_list[i].used == false) {
					empty_port = p_list[i].port;
					p_list[i].used = true;
					break;
				}
			}
			if (empty_port == 0) {
				send(AccSock, "not exist empty port", 21, 0);
				continue;
			}
			char hosting_msg[256];
			int host_index;
			for (int i = 0; i < 50; i++) {
				if (h_list[i] == 0) {
					host_index = i;
					break;
				}
			}
			sprintf(hosting_msg, "%04d%06d", empty_port, host::getLastHostNumber());
			send(AccSock, hosting_msg, 256, 0);
			h_list[host_index] = new host(host_index, identify, empty_port);
			cout << "포트 = " << empty_port << " 생성"<<endl;
			cout << "현재 호스트 수 = " << host::getHostCount() << endl << endl;
		}
		else if (str_msg[0] == '2')		// 호스트 참가
		{
			bool not_found = true;
			string identify = str_msg.substr(1, 10);
			identify = identify.substr(0, identify.find(" "));
			int host_number = stoi(str_msg.substr(11, 6));
			for (int i = 0; i < 50; i++) {
				if (h_list[i] == 0) continue;
				if (h_list[i]->getHostNumber() == host_number) {
					if ((h_list[i]->check_member_id(identify)) == -1) {
						send(AccSock, "I", 2, 0);
						not_found = false;
						break;
					}
					if (h_list[i]->getMemberCount() == 3 || h_list[i]->getIsStart() || h_list[i]->getIsClosed()) {
						send(AccSock, "F", 2, 0);
					}
					else {
						char stmp[256];
						string partici_msg = "\0";
						sprintf(stmp, "%04d", h_list[i]->getPort());
						partici_msg += stmp;
						for (int j = 0; j < 3; j++) {
							if ((h_list[i]->getMemberID(j))=="\0") {
								sprintf(stmp, "%d", j);
								partici_msg += stmp;
								break;
							}
						}
						for (int j = 0; j < 3; j++) {
							if ((h_list[i]->getMemberID(j)) == "\0")
								sprintf(stmp, "%-10s", "EMPTY");
							else
								sprintf(stmp, "%-10s", &(h_list[i]->getMemberID(j)[0]));
							partici_msg += stmp;
							if (h_list[i]->getMemberReady(j))
								partici_msg += 'R';
							else
								partici_msg += 'N';
						}
						send(AccSock, &partici_msg[0], 256, 0);
						h_list[i]->addMember(identify);
					}
					not_found = false;
					break;
				}
			}
			if(not_found)
				send(AccSock, "F", 2, 0);
		}
		closesocket(AccSock);
	}

	closesocket(ConSock);
}