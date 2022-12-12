#define _CRT_SECURE_NO_WARNINGS // ���� C �Լ� ��� �� ��� ����
#define _WINSOCK_DEPRECATED_NO_WARNINGS // ���� ���� API ��� �� ��� ����

#include <winsock2.h> // ����2 ���� ���
#include <ws2tcpip.h> // ����2 Ȯ�� ���
#include <stdio.h> // printf(), ...
#include "resource.h"
#include "GameFrame.h"

#pragma comment(lib, "ws2_32") // ws2_32.lib ��ũ

#define NICKBUFSIZE 21

DWORD WINAPI roomServerThread(LPVOID arg);
DWORD WINAPI roomClientThread(LPVOID arg);
DWORD WINAPI roomDataProcessingThread(LPVOID arg);
DWORD WINAPI roomDataResendThread(LPVOID arg);

DWORD WINAPI inGameServerThread(LPVOID arg);
DWORD WINAPI inGameClientThread(LPVOID arg);
DWORD WINAPI inGameClientResendThread(LPVOID arg);



class WAITING_ROOM
{
public:
	WAITING_ROOM();
	~WAITING_ROOM();
	WAITING_ROOM(const WAITING_ROOM& wr);
	int MAKE_ROOM(char* serverport);
	int CONNECT_ROOM(char* serverip, char* name, char* serverport);

	int		FindBlankPlayer();
	bool	checkNickReduplication(char* name);
	bool	checkJoin(char* name);
	int		stringAnalysis(char* recvdata);

	void	pressReady();
	bool	checkAllReady();
	void	pressStart();

	void	enableConnectGui(bool isEnable);
	void	printErrorEditbox(char* errstr);


	int			GetMyNum();
	void		SetMyNum(int in);
	SOCKET		GetMySock();
	void		SetMySock(SOCKET in);
	HWND		GetDlgHandle();
	void		SetDlgHandle(HWND in);
	bool		GetIsHost();
	int			GetPlayerCount();

	bool		GetIsIngame();
	void		SetIsIngame(bool in);


private:
	WSADATA				wsa;
	SOCKET				my_sock;
	struct sockaddr_in	serveraddr;

	bool				is_ingame{ false };
	bool				is_ready{ false };
	bool				is_host{ false };
	int					my_num{ -1 };
	int					playerCount{ 2 }; //�� �÷��̾� ����(��������)
	HWND				DlgHandle;
};


class INGAME
{
public:
	INGAME();
	INGAME(SOCKET sock, int num);
	~INGAME();
	INGAME(const INGAME& ig);



	int			GetMyNum();
	SOCKET		GetMySock();
	bool		GetIsHost();
	void		SetIsHost(bool in);
	int			stringAnalysis(Player* op, char* recvdata);

private:
	SOCKET				my_sock;

	int					my_num{ -1 };
	bool				is_host{ false };
};

