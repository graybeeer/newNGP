#pragma once

#include "framework.h"
#include "Player.h"

class Stage {
public:
	Stage();
	Stage(const string& backGroundKey, const wstring& backGroundFileName);
	Stage(int stageNum, const wstring& backGroundFileName);
	Stage(const wstring& backGroundFileName);
	~Stage();

	class Player* m_player{ };
	list<class Player*> m_otherPlayerList{ }; //다른 멀티 플레이어 리스트
	list<class Monster*> m_monsterList{ };
private:
	int m_stageNum{ };
	CImage* m_backBuffer{ };
	pair<string, wstring> m_backBufferInfo{ };
	wstring m_backGroundFileName{ };
	string m_backGroundKey{ };

	
	list<class Barigate*> m_barigateList{ };
	list<class Bullet*> m_bulletList{ };
	vector<class Button*> m_buttonVec{ };
	class Portal* m_portal{ };
	class Boss* m_boss{ };

	bool m_playerInPortal{ false };

	bool m_freeCamera{ false };

public:
	int GetStageNum() const;
	void SetStageNum(int num);

public:
	void CreateBullet(const string& tag, const POINT& stPt, int moveDir, float angle = 0);

public:
	void Update(float deltaTime);
	void Input(float deltaTime);
	void Render(HDC mainDC);

	void CollisionCheck(float deltaTime);
	void DeleteDeadObject();

	void CalCameraPos();
	void CalCameraPos(const POINT& mousePt);
	void CameraMove(float deltaTime, int moveDir);

	bool Save(const string& strPath);
	bool Load(const string& fileName);

	void AddPlayer(class Player* player);
	void AddMonster(class Monster* monster);
	void AddBarigate(class Barigate* barigate);
	void AddPortal(class Portal* portal);
	void AddButton(class Button* button);

	void DeleteObjEdit(const POINT& dPt);
};