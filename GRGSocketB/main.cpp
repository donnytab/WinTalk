#include <WinSock2.h>
#include <stdio.h>

#pragma comment (lib, "Ws2_32.lib")

/*************************************************
 程序常量和全局数据
**************************************************/
const int BUF_SZ = 1024;
const int PORT = 3001;
const char* SOCKET_IP_ADDR = "127.0.0.1";
bool g_bRunSocket = true;

/*************************************************
函 数 名:   ThreadBProc
功能概要:   线程B函数，作为服务器端通过套接字完成数据收发
**************************************************/
DWORD WINAPI ThreadBProc(LPVOID param)
{
	// 初始化Winsock
	WSADATA wsaData;
	int iRet = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iRet)
	{
		printf("[ERR] Thread B: Failed to start! ErrorCode(%d)\n", iRet);
		return iRet;
	}
	printf("[LOG] Thread B: WSA init successful\n");

	//创建TCP套接字
	SOCKET sSocketListen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	
	// 初始化套接字地址结构体
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = PORT;
	addr.sin_addr.S_un.S_addr = inet_addr(SOCKET_IP_ADDR);

	// 绑定套接字
	iRet = bind(sSocketListen, (sockaddr*)&addr, sizeof(addr));
	if (SOCKET_ERROR == iRet)
	{
		printf("[ERR] Thread B: Failed to bind! ErrorCode(%d)\n", iRet);
		closesocket(sSocketListen);
		WSACleanup();
		return 1;
	}
	printf("[LOG] Thread B: socket bind successful\n");

	// 监听套接字
	iRet = listen(sSocketListen, SOMAXCONN);
	if (SOCKET_ERROR == iRet)
	{
		printf("[ERR] Thread B: Failed to listen! ErrorCode(%d)\n", iRet);
		closesocket(sSocketListen);
		WSACleanup();
		return 1;
	}
	printf("[LOG] Thread B: socket listening...\n");

	// 接受客户端程序A的连接请求
	SOCKET sSocketConn = accept(sSocketListen, nullptr, nullptr);
	if (INVALID_SOCKET == sSocketConn)
	{
		printf("[ERR] Thread B: Failed to accept! ErrorCode(%d)\n", iRet);
		closesocket(sSocketListen);
		WSACleanup();
		return 1;
	}
	printf("[LOG] Thread B: socket accept successful\n");

	// 初始化套接字接收缓存
	char arrRecvBuf[BUF_SZ];
	memset(arrRecvBuf, 0, sizeof(arrRecvBuf));
	while(g_bRunSocket)
	{
		// 接收程序A发回的数据
		iRet = recv(sSocketConn, arrRecvBuf, sizeof(arrRecvBuf), 0);
		if (iRet > 0)
		{
			printf("[LOG] Thread B: Receive message from socket :%s\n", arrRecvBuf);
			// 向程序A发送接收到的数据
			iRet = send(sSocketConn, arrRecvBuf, sizeof(arrRecvBuf), 0);
			if (iRet > 0)
			{
				printf("[LOG] Thread B: Send message to socket :%s\n", arrRecvBuf);
			}
		}
	}

	shutdown(sSocketConn, SD_SEND);

	// 关闭套接字，结束Winsock
	closesocket(sSocketListen);
	closesocket(sSocketConn);
	WSACleanup();
	printf("[LOG] Thread B: Cleanup finished\n");
	return 0;
}

int main(void) {
	DWORD dwThreadIdB = 2;
	// 创建线程B
	HANDLE hThreadBHdl = CreateThread(nullptr, 0, ThreadBProc, nullptr, 0, &dwThreadIdB);
	if (hThreadBHdl)
	{
		printf("[LOG] Thread B creation successful\n");
		WaitForSingleObject(hThreadBHdl, INFINITE);
	}
	else
	{
		printf("[ERR] Failed to create Thread B\n");
	}
	// 暂停显示
	system("pause");
	return 0;
}