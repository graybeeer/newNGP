#include "Server.h"

DWORD WINAPI roomServerThread(LPVOID arg)
{
	WAITING_ROOM wr_server = *(WAITING_ROOM*)arg;
	SOCKET client_sock;
	SOCKET server_sock = wr_server.GetMySock();
	struct sockaddr_in clientaddr;
	int addrlen = sizeof(clientaddr);
	HANDLE hnd;

	while (1) {
		client_sock = accept(server_sock, (struct sockaddr*)&clientaddr, &addrlen);
		short cl_num = wr_server.FindBlankPlayer();
		if (cl_num == -1) {
			closesocket(client_sock);
			continue;
		};
		wr_server.SetMyNum(cl_num);
		wr_server.SetMySock(client_sock);
		hnd = CreateThread(NULL, 0, roomDataProcessingThread, (LPVOID)&wr_server, 0, NULL);
		CloseHandle(hnd);
	}

	return 0;
}

DWORD WINAPI roomClientThread(LPVOID arg)
{
	WAITING_ROOM wr_server = *(WAITING_ROOM*)arg;

	SOCKET sv_sock = wr_server.GetMySock();
	HWND hDlg = wr_server.GetDlgHandle();
	char recvcode[30];
	int strcode;

	int retval = recv(sv_sock, recvcode, NICKBUFSIZE, MSG_WAITALL);
	SetDlgItemTextA(hDlg, IDC_HOSTNAME, recvcode); // IDC_P1NAME + n = IDC_P(n+1)NAME

	while (1) {
		retval = recv(sv_sock, recvcode, 3, MSG_WAITALL);

		// ��쿡 ���� ��ȣ�ۿ� ����
		
		strcode = wr_server.stringAnalysis(recvcode);
		if (strcode == -1 || retval <= 0) {
			break;
		}
		else if (strcode == -2) {
			return 0;
		}
	}

	// �������� ���� ����� ó��
	for (int i{}; i < 4; ++i)
		SetDlgItemTextA(hDlg, IDC_HOSTNAME + i, ""); // IDC_P1NAME + n = IDC_P(n+1)NAME
	wr_server.enableConnectGui(TRUE);
	closesocket(sv_sock);
	return 0;
}

DWORD WINAPI roomDataProcessingThread(LPVOID arg)
{
	struct WAITING_ROOM wr_server = *(struct WAITING_ROOM*)arg;

	short cl_num = wr_server.GetMyNum();
	SOCKET cl_sock = wr_server.GetMySock();
	HWND hDlg = wr_server.GetDlgHandle();
	char recvcode[30];
	int retval = 0;
	int strcode;

	while (1) {
		int retval = recv(cl_sock, recvcode, 3, MSG_WAITALL);

		// ��쿡 ���� ��ȣ�ۿ� ����
		strcode = wr_server.stringAnalysis(recvcode);
		if (strcode == -1 || retval <= 0) {
			break;
		}
		else if (strcode == -2) {
			return 0;
		}
	}

	// Ŭ���̾�Ʈ cl_num���� ���� ����� ó��
	SetDlgItemTextA(hDlg, IDC_P1NAME + cl_num, ""); // IDC_P1NAME + n = IDC_P(n+1)NAME
	closesocket(cl_sock);
	return 0;
}

DWORD WINAPI roomDataResendThread(LPVOID arg)
{
	struct WAITING_ROOM wr_server = *(struct WAITING_ROOM*)arg;

	short cl_num = wr_server.GetMyNum();
	SOCKET cl_sock = wr_server.GetMySock();
	HWND hDlg = wr_server.GetDlgHandle();
	char tmpbuf[NICKBUFSIZE];
	char tmpstr[2];
	int retval = 0;

	//�÷��̾� ������ ���� ���ڿ�
	char tmpstr_p[11];
	char coordbuf_p[11];
	char tmpbuf_p[5];

	char coordbuf_m[20];

	bool beforeReadyStatus[3]; // ���� ������¿� ���Ͽ� dlg�� ��ȭ�� ������ �������ϴ� ����
	for (int i{}; i < 3; ++i) {
		GetDlgItemTextA(hDlg, IDC_P1READY + i, tmpbuf, 10);
		beforeReadyStatus[i] = (strcmp(tmpbuf, "") == 0) ? false : true;
	}
	
	// Host Nickname
	GetDlgItemTextA(hDlg, IDC_EDITNICKNAME, tmpbuf, NICKBUFSIZE);
	send(cl_sock, tmpbuf, NICKBUFSIZE, 0);

	while (1) {
		// ��й�
		if (!wr_server.GetIsIngame()) {
			for (int i{}; i < 3; ++i) {
				// i�� �÷��̾��� �г��� ������ �ٲ���� ����� Ŭ���̾�Ʈ���� ����
				GetDlgItemTextA(hDlg, IDC_P1NAME + i, tmpbuf, NICKBUFSIZE);
				_itoa(i, tmpstr, 10);
				retval = send(cl_sock, (char*)"NN", 3, 0);
				retval = send(cl_sock, tmpstr, 2, 0);
				retval = send(cl_sock, tmpbuf, NICKBUFSIZE, 0);

				// i�� �÷��̾��� ���� ������ �ٲ���� ����� Ŭ���̾�Ʈ���� ����
				GetDlgItemTextA(hDlg, IDC_P1READY + i, tmpbuf, 10);
				bool dlgReady = (strcmp(tmpbuf, "") == 0) ? false : true;
				if (beforeReadyStatus[i] != dlgReady) {
					beforeReadyStatus[i] = !beforeReadyStatus[i];
					retval = send(cl_sock, (char*)"RD", 3, 0);
					retval = send(cl_sock, tmpstr, 2, 0);
				}
			}

			GetDlgItemTextA(hDlg, IDC_HOSTREADY, tmpbuf, NICKBUFSIZE);
			if (strcmp(tmpbuf, "Starting...") == 0) {
				retval = send(cl_sock, (char*)"ST", 3, 0);
				GetDlgItemTextA(hDlg, IDC_P1READY + cl_num, tmpbuf, NICKBUFSIZE);
				SetDlgItemTextA(hDlg, IDC_P1READY + cl_num, (strcmp(tmpbuf, "") == 0) ? "" : "Starting...");
			}

			// ���� ���� ��(Starting)�� �� ���� �÷��̾��� ��, ����(Start)ó��
			bool checkWrongAccess = false;
			for (int i{}; i < cl_num; ++i) {
				GetDlgItemTextA(hDlg, IDC_P1NAME + i, tmpbuf, NICKBUFSIZE);
				if (strcmp(tmpbuf, "") != 0) {
					GetDlgItemTextA(hDlg, IDC_P1READY + i, tmpbuf, NICKBUFSIZE);
					if (strcmp(tmpbuf, "Starting...") == 0) {
						checkWrongAccess = true;
						break;
					}
				}
			}
			GetDlgItemTextA(hDlg, IDC_P1READY + cl_num, tmpbuf, NICKBUFSIZE);
			if (!checkWrongAccess && strcmp(tmpbuf, "Starting...") == 0) {
				// ���� ó��
				send(cl_sock, (char*)"CR", 3, 0);
				SetDlgItemTextA(hDlg, IDC_P1READY + cl_num, "Start!");
			}

			// ��� �÷��̾ ���� ������ �� Dlg�� ���� �ΰ�������
			bool goStart = false;
			for (int i{}; i < 3; ++i) {
				GetDlgItemTextA(hDlg, IDC_P1NAME + i, tmpbuf, NICKBUFSIZE);
				if (strcmp(tmpbuf, "") != 0) {
					GetDlgItemTextA(hDlg, IDC_P1READY + i, tmpbuf, NICKBUFSIZE);
					if (strcmp(tmpbuf, "Start!") == 0) {
						goStart = true;
					}
					else {
						goStart = false;
						break;
					}
				}
			}
			if (goStart) {
				wr_server.SetIsIngame(true);
				EndDialog(hDlg, 0);
			}
		}
		else {
			if (gameFrame.m_curStage->m_player) {
				
				tmpstr_p[0] = '\0';

				send(cl_sock, "CO", 3, 0);
				POINT pt = gameFrame.m_curStage->m_player->GetPlayerPt();
				strcpy(coordbuf_p, "0000");
				
				_itoa(pt.x, tmpbuf_p, 10);
				int lentmp = strlen(tmpbuf_p);
				for (int i{}; i < lentmp; ++i) {
					coordbuf_p[(4 - lentmp) + i] = tmpbuf_p[i];
				}
				send(cl_sock, coordbuf_p, 5, 0);

				strcpy(coordbuf_p, "0000");
				_itoa(pt.y, tmpbuf_p, 10);
				lentmp = strlen(tmpbuf_p);
				for (int i{}; i < lentmp; ++i) {
					coordbuf_p[(4 - lentmp) + i] = tmpbuf_p[i];
				}
				send(cl_sock, coordbuf_p, 5, 0);

				//���� ����Ʈ ���� ��ġ ������
				send(cl_sock, "MO", 3, 0);
				auto firstMonster = gameFrame.m_curStage->m_monsterList.begin();
				for (firstMonster; firstMonster != gameFrame.m_curStage->m_monsterList.end(); firstMonster++)
				{
					Monster* monster = *firstMonster;
					POINT pt_m = monster->GetMonsterPt();
					
					sprintf(coordbuf_m, "%5d%5d", pt_m.x, pt_m.y);
					send(cl_sock, coordbuf_m, 10, 0);
				}
			}
		}

		if (retval == SOCKET_ERROR) {
			break;
		}
		if (!wr_server.GetIsIngame()) 
		{
			Sleep(333); //�����϶� 333����
		}
		else
		{
			Sleep(6); //�ΰ����̸� 6����
		}
	}

	return 0;
}

DWORD WINAPI inGameServerThread(LPVOID arg)
{
	INGAME ig_server = *(INGAME*)arg;
	ig_server.SetIsHost(true);
	SOCKET cl_sock = ig_server.GetMySock();
	char recvco[30];
	memset(recvco, 1, 30);
	int retval;

	auto opIter = gameFrame.m_curStage->m_otherPlayerList.begin();
	send(cl_sock, "ST", 3, 0);

	while (1) {
		retval = recv(cl_sock, recvco, 3, MSG_WAITALL);

		// ��쿡 ���� ��ȣ�ۿ� ����
		if (ig_server.stringAnalysis(*opIter, recvco) == -1 || retval <= 0)
			break;
	}

	return 0;
}

DWORD WINAPI inGameClientThread(LPVOID arg)
{
	INGAME ig_server = *(INGAME*)arg;

	SOCKET sv_sock = ig_server.GetMySock();
	char recvcode[30];

	int retval;


	while (1) {
		retval = recv(sv_sock, recvcode, 3, MSG_WAITALL);

		auto opIter = gameFrame.m_curStage->m_otherPlayerList.begin();
		// ��쿡 ���� ��ȣ�ۿ� ����
		if (ig_server.stringAnalysis(*opIter, recvcode) == -1) {
			break;
		}

	}
	return 0;
}


DWORD WINAPI inGameClientResendThread(LPVOID arg)
{
	WAITING_ROOM ig_client = *(WAITING_ROOM*)arg;

	SOCKET ig_sock = ig_client.GetMySock();
	char recvcode[30];

	int retval;
	while (1) {
		if (gameFrame.m_curStage->m_player) {
			char tmpstr[11];
			char coordbuf[11];
			tmpstr[0] = '\0';

			send(ig_sock, "CO", 3, 0);
			POINT pt = gameFrame.m_curStage->m_player->GetPlayerPt();
			strcpy(coordbuf, "0000");
			char tmpbuf[5];
			_itoa(pt.x, tmpbuf, 10);
			int lentmp = strlen(tmpbuf);
			for (int i{}; i < lentmp; ++i) {
				coordbuf[(4 - lentmp) + i] = tmpbuf[i];
			}
			send(ig_sock, coordbuf, 5, 0);

			strcpy(coordbuf, "0000");
			_itoa(pt.y, tmpbuf, 10);
			lentmp = strlen(tmpbuf);
			for (int i{}; i < lentmp; ++i) {
				coordbuf[(4 - lentmp) + i] = tmpbuf[i];
			}
			send(ig_sock, coordbuf, 5, 0);
		}
		Sleep(6);
	}

	return 0;
}

WAITING_ROOM::WAITING_ROOM()
{
	WSAStartup(MAKEWORD(2, 2), &wsa);
}

WAITING_ROOM::~WAITING_ROOM()
{
}

WAITING_ROOM::WAITING_ROOM(const WAITING_ROOM& wr)
{
	wsa = wr.wsa;
	my_sock = wr.my_sock;
	serveraddr = wr.serveraddr;

	is_ready = wr.is_ready;
	is_host = wr.is_host;
	my_num = wr.my_num;
	DlgHandle = wr.DlgHandle;
}

int WAITING_ROOM::MAKE_ROOM(char* serverport)
{
	my_num = -2;
	is_host = true;
	int retval = 0;
	my_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (my_sock == SOCKET_ERROR)
		return -1;

	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(atoi(serverport));
	retval = bind(my_sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR)
		return -1;

	listen(my_sock, SOMAXCONN);
	if (retval == SOCKET_ERROR)
		return -1;

	HANDLE hnd = CreateThread(NULL, 0, roomServerThread, (LPVOID)this, 0, NULL);
	CloseHandle(hnd);
	return 0;
}

int WAITING_ROOM::CONNECT_ROOM(char* serverip, char* name, char* serverport)
{
	my_num = -2;
	is_host = false;
	my_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (my_sock == INVALID_SOCKET)
		return -2;

	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	inet_pton(AF_INET, serverip, &serveraddr.sin_addr);
	serveraddr.sin_port = htons(atoi(serverport));
	if (connect(my_sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) == SOCKET_ERROR)
		return -2;

	send(my_sock, (char*)"NN", 3, 0);
	send(my_sock, name, NICKBUFSIZE, 0);

	char tmpbuf[2];
	recv(my_sock, tmpbuf, 2, MSG_WAITALL);

	if (strcmp(tmpbuf, "X") == 0) {
		return -1;
	}

	HANDLE hnd = CreateThread(NULL, 0, roomClientThread, (LPVOID)this, 0, NULL);
	CloseHandle(hnd);

	return 0;
}

int WAITING_ROOM::FindBlankPlayer()
{
	char tmpname[NICKBUFSIZE];
	for (int i{}; i < 3; ++i) {
		GetDlgItemTextA(DlgHandle, IDC_P1NAME + i, tmpname, 20);
		if (strcmp(tmpname, "") == 0)
			playerCount = i + 1;
			return i;
	}
	playerCount = 4;
	return -1;
}

// ������ �ִ� �г������� �˻�
bool WAITING_ROOM::checkNickReduplication(char* name)
{
	char tmpname[NICKBUFSIZE];
	for (int i{}; i < 4; ++i) {
		GetDlgItemTextA(DlgHandle, IDC_HOSTNAME + i, tmpname, NICKBUFSIZE);
		if (strcmp(name, tmpname) == 0) {
			return true;
		}
	}
	return false;
}

// �������� ���� �õ����� �˻�
bool WAITING_ROOM::checkJoin(char* name)
{
	bool isAlreadyExist = checkNickReduplication(name);
	if (isAlreadyExist) {
		send(my_sock, "X", 2, 0); // �г��� �ߺ��̶�� ���� Ŭ���̾�Ʈ ������ ����
		return false;
	}

	send(my_sock, "O", 2, 0); // ���������� �����Ͽ��ٴ� ���� Ŭ���̾�Ʈ ������ ����
	HANDLE hnd = CreateThread(NULL, 0, roomDataResendThread, (LPVOID)this, 0, NULL);
	CloseHandle(hnd);
	return true;
}

int WAITING_ROOM::stringAnalysis(char* recvdata)
{
	int retval = 0;
	char recvcode[30];
	char tmpstr[2];

	// Host�� ����� �������� ó��
	if (is_host) {
		if (strcmp(recvdata, "NN") == 0) { // �г��� ���� ������ ���
			recv(my_sock, recvcode, NICKBUFSIZE, MSG_WAITALL);
			if (checkJoin(recvcode))
				SetDlgItemTextA(DlgHandle, IDC_P1NAME + my_num, recvcode); // IDC_P1NAME + n = IDC_P(n+1)NAME
			else 
				return -1; // �г��� �ߺ�
		}
		else if (strcmp(recvdata, "RD") == 0) { // Ready ���� ������ ���
			GetDlgItemTextA(DlgHandle, IDC_P1READY + my_num, recvcode, 10);
			SetDlgItemTextA(DlgHandle, IDC_P1READY + my_num, (strcmp(recvcode, "") == 0) ? "Ready!" : "");
		}		
		else if (strcmp(recvdata, "CO") == 0) { // ��ǥ ���� ������ ���

			POINT pt;
			recv(my_sock, recvcode, 5, MSG_WAITALL);
			pt.x = atoi(recvcode);

			recv(my_sock, recvcode, 5, MSG_WAITALL);
			pt.y = atoi(recvcode);

			auto opIter = gameFrame.m_curStage->m_otherPlayerList.begin();
			if ((*opIter)) {
				(*opIter)->SetPt(pt);
			}
		}
	}
	// Client�� ����� �������� ó��
	else {
		if (strcmp(recvdata, "NN") == 0) {
			recv(my_sock, recvcode, 2, MSG_WAITALL);
			int editnum = atoi(recvcode);
			recv(my_sock, recvcode, NICKBUFSIZE, MSG_WAITALL);
			SetDlgItemTextA(DlgHandle, IDC_P1NAME + editnum, recvcode); // IDC_P1NAME + n = IDC_P(n+1)NAME
		}
		else if (strcmp(recvdata, "RD") == 0) { // Ready ���� ������ ���
			recv(my_sock, recvcode, 2, MSG_WAITALL);
			int editnum = atoi(recvcode);
			char tmpstr[10];
			GetDlgItemTextA(DlgHandle, IDC_P1READY + editnum, tmpstr, 10);
			SetDlgItemTextA(DlgHandle, IDC_P1READY + editnum, (strcmp(tmpstr, "") == 0) ? "Ready!" : "");
		}
		else if (strcmp(recvdata, "ST") == 0) {
			pressStart();
		}
		else if (strcmp(recvdata, "CO") == 0) {
			/*
			recv(my_sock, recvcode, 2, MSG_WAITALL);
			int editnum = atoi(recvcode);
			*/
			POINT pt;
			recv(my_sock, recvcode, 5, MSG_WAITALL);
			pt.x = atoi(recvcode);

			recv(my_sock, recvcode, 5, MSG_WAITALL);
			pt.y = atoi(recvcode);

			auto opIter = gameFrame.m_curStage->m_otherPlayerList.begin();
			if ((*opIter)) {
				(*opIter)->SetPt(pt);
			}
		}
		else if (strcmp(recvdata, "MO") == 0) {
			auto monster = gameFrame.m_curStage->m_monsterList.begin();
			for (monster; monster != gameFrame.m_curStage->m_monsterList.end(); monster++)
			{
				Monster* mon = *monster;
				POINT pt;
				recv(my_sock, recvcode, 5, MSG_WAITALL);
				pt.x = atoi(recvcode);

				recv(my_sock, recvcode, 5, MSG_WAITALL);
				pt.y = atoi(recvcode);
				mon->SetPt(pt);
			}
			

		}
		else if (strcmp(recvdata, "CR") == 0) {
			//Ŭ���̾�Ʈ�� �ΰ����� �����ϸ� inGameClientResendThread�� ����
			FindBlankPlayer(); //������ �����ϱ��� �ο�üũ
			EndDialog(DlgHandle, 0);
			HANDLE hnd = CreateThread(NULL, 0, inGameClientResendThread, (LPVOID)this, 0, NULL);
			CloseHandle(hnd);
		}
	}

	return 0;
}

void WAITING_ROOM::pressReady()
{
	if (!is_host){
		is_ready = !is_ready;
		send(my_sock, "RD", 3, 0);
	}
}

bool WAITING_ROOM::checkAllReady()
{
	char tmpstr[10];
	for (int i{}; i < 3; ++i) {
		GetDlgItemTextA(DlgHandle, IDC_P1NAME + i, tmpstr, 10);
		if (strcmp(tmpstr, "") != 0) {
			GetDlgItemTextA(DlgHandle, IDC_P1READY + i, tmpstr, 10);
			if (strcmp(tmpstr, "") == 0)
				return false;
		}
	}
	return true;
}

void WAITING_ROOM::pressStart()
{
	if (checkAllReady()){
		char tmpstr[30];

		SetDlgItemTextA(DlgHandle, IDC_HOSTREADY, "Starting...");
		if (!is_host) {
			for (int i{}; i < 3; ++i) {
				GetDlgItemTextA(DlgHandle, IDC_P1READY + i, tmpstr, 10);
				SetDlgItemTextA(DlgHandle, IDC_P1READY + i, (strcmp(tmpstr, "") == 0) ? "" : "Starting...");
			}
		}
	}
}

void WAITING_ROOM::enableConnectGui(bool isEnable)
{
	HWND HwndEditNickname = GetDlgItem(DlgHandle, IDC_EDITNICKNAME);
	HWND HwndIpaddress = GetDlgItem(DlgHandle, IDC_IPADDRESS);
	HWND HwndServerPort = GetDlgItem(DlgHandle, IDC_EDIT_SERVERPORT);
	HWND HwndMakeroom = GetDlgItem(DlgHandle, IDC_MAKEROOM);
	HWND HwndConnectroom = GetDlgItem(DlgHandle, IDC_CONNECTROOM);
	
	EnableWindow(HwndMakeroom, isEnable);
	EnableWindow(HwndIpaddress, isEnable);
	EnableWindow(HwndServerPort, isEnable);
	EnableWindow(HwndEditNickname, isEnable);
	EnableWindow(HwndConnectroom, isEnable);
}

void WAITING_ROOM::printErrorEditbox(char* errstr)
{
	char warnbuf[40];
	strcpy(warnbuf, errstr);
	SetDlgItemTextA(DlgHandle, IDC_EDITNICKNAME, warnbuf);
}

int WAITING_ROOM::GetMyNum()
{
	return my_num;
}

void WAITING_ROOM::SetMyNum(int in)
{
	my_num = in;
}

SOCKET WAITING_ROOM::GetMySock()
{
	return my_sock;
}

void WAITING_ROOM::SetMySock(SOCKET in)
{
	my_sock = in;
}

HWND WAITING_ROOM::GetDlgHandle()
{
	return DlgHandle;
}

void WAITING_ROOM::SetDlgHandle(HWND in)
{
	DlgHandle = in;
}

bool WAITING_ROOM::GetIsHost()
{
	return is_host;
}

int WAITING_ROOM::GetPlayerCount()
{
	return playerCount;
}

bool WAITING_ROOM::GetIsIngame()
{
	return is_ingame;
}

void WAITING_ROOM::SetIsIngame(bool in)
{
	is_ingame = in;
}




INGAME::INGAME()
{
}

INGAME::INGAME(SOCKET sock, int num)
{
	my_sock = sock;
	my_num = num;
}

INGAME::~INGAME()
{
}

INGAME::INGAME(const INGAME& ig)
{
	my_sock = ig.my_sock;
	my_num = ig.my_num;
	is_host = ig.is_host;
}

int INGAME::GetMyNum()
{
	return my_num;
}

SOCKET INGAME::GetMySock()
{
	return my_sock;
}

bool INGAME::GetIsHost()
{
	return is_host;
}

void INGAME::SetIsHost(bool in)
{
	is_host = in;
}

int INGAME::stringAnalysis(Player* op, char* recvdata)
{
	int retval = 0;
	char recvcode[30];
	char tmpstr[3];

	// Host�� ����� �������� ó��
	if (is_host) {
	}
	// Client�� ����� �������� ó��
	else {

	}

	return 0;
}
