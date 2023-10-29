#include <WinSock2.h>
#include <stdio.h>

#pragma comment (lib, "Ws2_32.lib")

/*************************************************
 程序常量和全局数据
**************************************************/
const int BUF_SZ = 1024;    // 缓存大小
const int PORT = 3001;      // 端口号
const char* SOCKET_IP_ADDR = "127.0.0.1";     // 套接字IP地址
const char* DATA_FORMAT = "%08d\0";          // 8位数字指令
const unsigned int IPC_TERMINATION_THRESHOLD = 99999999;   // 数据指令上限
unsigned int g_uiData = 1;     // 数据指令
bool g_bRunSocket = true;      // 运行标记

/*************************************************
函 数 名:   ThreadAProc
功能概要:   线程A函数，作为客户端通过套接字完成数据收发
**************************************************/
DWORD WINAPI ThreadAProc(LPVOID param)
{
	// 初始化Winsock
	WSADATA wsaData;
	int iRet = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iRet)
	{
		printf("[ERR] Thread A: Failed to init! ErrorCode(%d)\n", iRet);
		return iRet;
	}
	printf("[LOG] Thread A: WSA init successful\n");

	// 创建TCP套接字
	SOCKET sSocketConn = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	
	// 初始化套接字地址结构体
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = PORT;
	addr.sin_addr.S_un.S_addr = inet_addr(SOCKET_IP_ADDR);

	// 连接套接字
	iRet = connect(sSocketConn, (sockaddr*)&addr, sizeof(addr));
	if (SOCKET_ERROR == iRet)
	{
		printf("[ERR] Thread A: Failed to connect! ErrorCode(%d)\n", iRet);
		closesocket(sSocketConn);
		WSACleanup();
		return 1;
	}
	printf("[LOG] Thread A: socket connect successful\n");

	// 初始化套接字发送缓存和接收缓存
	char arrSendBuf[BUF_SZ];
	char arrRecvBuf[BUF_SZ];
	memset(arrSendBuf, 0, sizeof(arrSendBuf));
	memset(arrRecvBuf, 0, sizeof(arrRecvBuf));
	
	while(g_bRunSocket)
	{
		// 将数据指令写入发送缓存
		sprintf(arrSendBuf, DATA_FORMAT, g_uiData);

		// 向服务器端程序B发送数据
		iRet = send(sSocketConn, arrSendBuf, sizeof(arrSendBuf), 0);
		if (iRet > 0)
		{
			printf("[LOG] Thread A: Send message to socket :%s\n", arrSendBuf);
		}

		// 接收服务器端程序B发回的数据
		iRet = recv(sSocketConn, arrRecvBuf, sizeof(arrRecvBuf), 0);
		if (iRet > 0)
		{
			printf("[LOG] Thread A: Receive message from socket :%s\n", arrRecvBuf);
		}

		// 判断接收的数据指令是否超出限制
		int iRecvVal = atoi(arrRecvBuf);
		if (iRecvVal >= IPC_TERMINATION_THRESHOLD)
		{
			printf("[LOG] Thread A: Reach IPC termination threshold %d, current value:%d\n", IPC_TERMINATION_THRESHOLD, g_uiData);
			printf("[LOG] Thread A: IPC finished\n");
			break;
		}

		// 完成通信后数据指令增加1
		++g_uiData;
	}

	// 关闭套接字，结束Winsock
	closesocket(sSocketConn);
	WSACleanup();
	printf("[LOG] Thread A: Cleanup finished\n");
	return 0;
}

/*************************************************
函 数 名:   main
功能概要:   主函数，完成线程A的创建和运行
**************************************************/
int main(void)
{
	DWORD dwThreadIdA = 1;
	// 创建线程A
	HANDLE hThreadAHdl = CreateThread(nullptr, 0, ThreadAProc, nullptr, 0, &dwThreadIdA);
	if (hThreadAHdl)
	{
		printf("[LOG] Thread A: creation successful\n");
		WaitForSingleObject(hThreadAHdl, INFINITE);
	}
	else
	{
		printf("[ERR] Failed to create Thread A\n");
	}
	// 暂停显示
	system("pause");
	return 0;
}