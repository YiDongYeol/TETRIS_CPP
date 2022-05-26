#include "server.h"
int host::last_host_number = 1;
int host::host_count = 0;

int main() {
	WSADATA WSAdata;
	if (WSAStartup(WINSOCK_VERSION, &WSAdata) == -1) {
		return -1;
	}
	startServe();
	
	WSACleanup();
	return 0;
}