#include "Stage.h"
#include "Global.h"
#include "EffectManager.h"
#include "Barigate.h"
#include "Player.h"
#include "Monster.h"
#include "Boss.h"
#include "Slime.h"
#include "BadOrange.h"
#include "Portal.h"
#include "Button.h"
#include "Server.h"

Stage::Stage() {
#ifdef EDIT
	m_freeCamera = true;
	cameraPos.x = clientSize.cx / 2;
	cameraPos.y = clientSize.cy / 2;
#endif
}

Stage::Stage(const string& backGroundKey, const wstring& backGroundFileName) {
#ifdef EDIT
	m_freeCamera = true;
	cameraPos.x = clientSize.cx / 2;
	cameraPos.y = clientSize.cy / 2;
#endif
	m_backBuffer = new CImage;
	m_backGroundFileName = backGroundFileName;
	m_backBuffer->Load(m_backGroundFileName.c_str());
}

Stage::Stage(int stageNum, const wstring& backGroundFileName) {
#ifdef EDIT
	m_freeCamera = true;
	cameraPos.x = clientSize.cx / 2;
	cameraPos.y = clientSize.cy / 2;
#endif
	m_stageNum = stageNum;
	m_backBuffer = new CImage;
	m_backGroundFileName = backGroundFileName;
	m_backBuffer->Load(m_backGroundFileName.c_str());
}

Stage::Stage(const wstring& backGroundFileName) {
#ifdef EDIT
	m_freeCamera = true;
	cameraPos.x = clientSize.cx / 2;
	cameraPos.y = clientSize.cy / 2;
#endif
	m_backBuffer = new CImage;
	m_backGroundFileName = backGroundFileName;
	m_backBuffer->Load(m_backGroundFileName.c_str());
}

Stage::~Stage() {
	if (m_player) {
		delete m_player;
	}
	m_player = nullptr;

	if (m_boss) {
		delete m_boss;
	}
	m_boss = nullptr;

	list<Monster*>::iterator mIterEnd = m_monsterList.end();
	for (auto iter = m_monsterList.begin(); iter != mIterEnd; ++iter) {
		if ((*iter)) {
			delete (*iter);
		}
		(*iter) = nullptr;
	}

	list<Barigate*>::iterator bIterEnd = m_barigateList.end();
	for (auto iter = m_barigateList.begin(); iter != bIterEnd; ++iter) {
		if ((*iter)) {
			delete (*iter);
		}
		(*iter) = nullptr;
	}

	if (m_backBuffer) {
		delete m_backBuffer;
	}
	m_backBuffer = nullptr;
}

int Stage::GetStageNum() const {
	return m_stageNum;
}

void Stage::SetStageNum(int num) {
	m_stageNum = num;
}

void Stage::CreateBullet(const string& tag, const POINT& stPt, int moveDir, float Angle) {
	m_bulletList.push_back(new Bullet{ tag, stPt, moveDir, Angle });
}

void Stage::Update(float deltaTime) {
	keyInfo.Update(deltaTime);


	static bool once = false;
	if (once) {
		if (!m_player) {
			cameraPos = { clientSize.cx / 2, clientSize.cy / 2 };
		}
	}

	/*if (!once) {
		switch (m_stageNum) {
		case 1:
			PlaySound(L"BGM1.wav", NULL, SND_ASYNC | SND_FILENAME | SND_LOOP | SND_NODEFAULT);
			break;

		case 2:
			PlaySound(L"BGM2.wav", NULL, SND_ASYNC | SND_FILENAME | SND_LOOP | SND_NODEFAULT);
			break;
		}
	}*/

	if (m_backBuffer) {
		if (!once) {
			m_backBuffer->GetDC();
			m_backBuffer->ReleaseDC();
			once = !once;
		}
		delete m_backBuffer;
	}
	m_backBuffer = new CImage;
	m_backBuffer->Load(m_backGroundFileName.c_str());

	worldSize = { m_backBuffer->GetWidth(), m_backBuffer->GetHeight() };

	if (m_stageNum == 2) {
		if (!m_boss) {
			m_boss = new Boss{ POINT { worldSize.cx / 2, worldSize.cy / 2 } };
		}
	}

#ifndef EDIT
	CollisionCheck(deltaTime);

	if (m_player) {
		m_player->Update(deltaTime);
		CalCameraPos();
	}
	//다른 플레이어 업데이트
	list<Player*>::iterator otherPlayeriter = m_otherPlayerList.begin();
	list<Player*>::iterator otherPlayeriterEnd = m_otherPlayerList.end();
	for (; otherPlayeriter != otherPlayeriterEnd; ++otherPlayeriter) {
		(*otherPlayeriter)->Update(deltaTime);
	}

	if (m_boss) {
		if (m_player) {
			m_boss->Update(deltaTime, m_player->GetPlayerPt());
		}
	}

	list<Bullet*>::iterator blIterEnd = m_bulletList.end();
	for (auto iter = m_bulletList.begin(); iter != blIterEnd; ++iter) {
		if ((*iter)->GetTargetingMode()) {
			if ((*iter)->GetTag() == "Boss") {
				if (m_player) {
					(*iter)->Update(deltaTime, m_player->GetPlayerPt());
				}
			}
			else if ((*iter)->GetTag() == TAGPLAYER) {
				if (m_boss) {
					(*iter)->Update(deltaTime, m_boss->GetPt());
				}
			}
		}
		else {
			(*iter)->Update(deltaTime);
		}
	}

	list<Barigate*>::iterator bIterEnd = m_barigateList.end();
	for (auto iter = m_barigateList.begin(); iter != bIterEnd; ++iter) {
		(*iter)->Update(deltaTime);
	}

	list<Monster*>::iterator iter = m_monsterList.begin();
	list<Monster*>::iterator iterEnd = m_monsterList.end();
	for (; iter != iterEnd; ++iter) {
		(*iter)->Update(deltaTime);
	}

	auto btIterEnd = m_buttonVec.end();
	for (auto iter = m_buttonVec.begin(); iter != btIterEnd; ++iter) {
		(*iter)->Update(deltaTime);
	}

	if (m_portal) {
		m_portal->Update(deltaTime);
	}

	EffectManager::GetInst()->Update(deltaTime);
#else
	CalCameraPos();
#endif
}

void Stage::Input(float deltaTime) {
	if (m_stageNum == 0) {
		return;
	}

	if (m_playerInPortal) {
		if (keyInfo[VK_DOWN].down) {
			NextStage();
		}
	}

#ifndef EDIT
	if (m_stageNum == 3 or m_stageNum == 4) {
		for (int i = 0; i < MAX_BUFFER; ++i) {
			if (keyInfo[i].down) {
				PostQuitMessage(0);
			}
		}
	}

	if (m_stageNum == 1 or m_stageNum == 2) {
		if (m_player) {
			m_player->Input(deltaTime);
		}
	}
#endif

#ifdef _DEBUG
	if (keyInfo[VK_F1].down) {
		CreateBullet(TAGMUSHROOM, POINT{ 500, 500 }, MD_TARGET);
	}

	if (keyInfo[VK_CONTROL].pressed) {
		if (keyInfo[VK_F1].down) {
			CreateBullet(TAGMUSHROOM, POINT{ 500, 500 }, MD_ANGLE);
		}
	}

#endif
	if (keyInfo[VK_CONTROL].pressed) {
		if (keyInfo[VK_F5].down) {
			debuging = !debuging;
		}
	}

	if (m_freeCamera) {
		if (keyInfo[VK_LEFT].pressed) {
			CameraMove(deltaTime, MD_LEFT);
		}

		if (keyInfo[VK_RIGHT].pressed) {
			CameraMove(deltaTime, MD_RIGHT);
		}

		if (keyInfo[VK_UP].pressed) {
			CameraMove(deltaTime, MD_UP);
		}

		if (keyInfo[VK_DOWN].pressed) {
			CameraMove(deltaTime, MD_DOWN);
		}
	}
}

void Stage::Render(HDC mainDC) {
	if (m_stageNum == 3 or m_stageNum == 4) {
		if (!m_backBuffer) {
			m_backBuffer = new CImage{ };
			m_backBuffer->Load(m_backGroundFileName.c_str());
		}
	}

	list<Barigate*>::iterator bIterEnd = m_barigateList.end();
	for (auto iter = m_barigateList.begin(); iter != bIterEnd; ++iter) {
		(*iter)->Render(m_backBuffer->GetDC());
		m_backBuffer->ReleaseDC();
	}

	list<Monster*>::iterator mIterEnd = m_monsterList.end();
	for (auto iter = m_monsterList.begin(); iter != mIterEnd; ++iter) {
		(*iter)->Render(m_backBuffer->GetDC());
		m_backBuffer->ReleaseDC();
	}

	if (m_player) {
		m_player->Render(m_backBuffer->GetDC());
		m_backBuffer->ReleaseDC();
	}
	//다른 플레이어 렌더
	list<Player*>::iterator otherEnd = m_otherPlayerList.end();
	for (auto iter = m_otherPlayerList.begin(); iter != otherEnd; ++iter) {
		(*iter)->Render(m_backBuffer->GetDC());
		m_backBuffer->ReleaseDC();
	}

	if (m_boss) {
		m_boss->Render(m_backBuffer->GetDC());
		m_backBuffer->ReleaseDC();
	}

	list<Bullet*>::iterator blIterEnd = m_bulletList.end();
	for (auto iter = m_bulletList.begin(); iter != blIterEnd; ++iter) {
		(*iter)->Render(m_backBuffer->GetDC());
		m_backBuffer->ReleaseDC();
	}

	auto btIterEnd = m_buttonVec.end();
	for (auto iter = m_buttonVec.begin(); iter != btIterEnd; ++iter) {
		(*iter)->Render(m_backBuffer->GetDC());
		m_backBuffer->ReleaseDC();
	}

	if (m_portal) {
		m_portal->Render(m_backBuffer->GetDC());
		m_backBuffer->ReleaseDC();
	}

	EffectManager::GetInst()->Render(m_backBuffer->GetDC());
	m_backBuffer->ReleaseDC();

	BitBlt(mainDC, 0, 0, clientSize.cx, clientSize.cy, m_backBuffer->GetDC(),
		cameraPos.x - clientSize.cx / 2, cameraPos.y - clientSize.cy / 2, SRCCOPY);
	m_backBuffer->ReleaseDC();
}

void Stage::CollisionCheck(float deltaTime) {
#ifndef EDIT
	list<Monster*>::iterator mIterEnd = m_monsterList.end();
	list<Barigate*>::iterator bgIterEnd = m_barigateList.end();
	list<Bullet*>::iterator blIterEnd = m_bulletList.end();
	list<Player*>::iterator opIterEnd = m_otherPlayerList.end();

	for (auto iter = m_monsterList.begin(); iter != mIterEnd; ++iter) {
		RECT rcTemp{ };
		RECT pRC = m_player->GetPlayerRC();
		RECT mRC = (*iter)->GetMonsterRC();
		RECT pPvRC = m_player->GetPrevRC();
		ATTACKINFO atkInfo = m_player->GetAttackInfo();
		if (!(*iter)->Diying()) {
			if (IntersectRect(&rcTemp, &pRC, &mRC)) {
				if (pPvRC.bottom <= mRC.top) {
					(*iter)->Attacked(50, 1);
					m_player->ResetJumpPower();
				}
				else {
					if (!m_player->IsInvincible()) {
						POINT pPt = m_player->GetPlayerPt();
						POINT mPt = (*iter)->GetMonsterPt();
						int dir = mPt.x > pPt.x ? -1 : 1;
						m_player->Attacked(10, dir);
					}
				}
			}

		}

		if (!(*iter)->Diying()) {
			if (!(*iter)->IsInvincible()) {
				if (IntersectRect(&rcTemp, &atkInfo.attackBox, &mRC)) {
					POINT pPt = m_player->GetPlayerPt();
					POINT mPt = (*iter)->GetMonsterPt();


					int dir = mPt.x < pPt.x ? -1 : 1;
					(*iter)->Attacked(atkInfo, dir);
				}
			}
		}
	}

	mIterEnd = m_monsterList.end();

	RECT temp{};
	for (auto mIter = m_monsterList.begin(); mIter != mIterEnd; ++mIter) {
		RECT mRC = (*mIter)->GetMonsterRC();
		for (auto bgIter = m_barigateList.begin(); bgIter != bgIterEnd; ++bgIter) {
			RECT bgRC = (*bgIter)->GetRC();
			RECT mPvRC = (*mIter)->GetPrevRC();
			if (IntersectRect(&temp, &mRC, &bgRC)) {
				if (mPvRC.bottom <= bgRC.top) {
					(*mIter)->OnGround(bgRC.top, bgRC);
				}
			}
			else {
				bgRC.top -= 1;
				if (IntersectRect(&temp, &mRC, &bgRC)) {
					if (mPvRC.bottom <= bgRC.top + 1) {
						(*mIter)->OnGround();
					}
				}
			}
		}
	}

	for (auto bgIter = m_barigateList.begin(); bgIter != bgIterEnd; ++bgIter) {
		RECT bgRC = (*bgIter)->GetRC();
		RECT pRC = m_player->GetPlayerRC();
		RECT pPvRC = m_player->GetPrevRC();
		if (IntersectRect(&temp, &pRC, &bgRC)) {
			if (pPvRC.bottom <= bgRC.top) {
				m_player->OnGround(bgRC.top);
			}
			else {
				if (pPvRC.top >= bgRC.bottom) {
					m_player->SetPower(0);
				}

				SIZE boxSize = m_player->GetHitBoxSize();
				if (pPvRC.left >= bgRC.right) {
					m_player->SetX(bgRC.right + boxSize.cx / 2);
				}
				else if (pPvRC.right <= bgRC.left) {
					m_player->SetX(bgRC.left - boxSize.cx / 2);
				}
			}
		}
		else {
			RECT tmp = bgRC;
			tmp.top -= 1;
			if (IntersectRect(&temp, &pRC, &tmp)) {
				if (pPvRC.bottom <= tmp.top + 1) {
					m_player->OnGround();
				}
			}
		}
	}

	for (auto bgIter = m_barigateList.begin(); bgIter != bgIterEnd; ++bgIter) {
		RECT bgRC = (*bgIter)->GetRC();
		for (auto opIter = m_otherPlayerList.begin(); opIter != opIterEnd; ++opIter) {
			RECT opRC = (*opIter)->GetPlayerRC();
			RECT opPvRC = (*opIter)->GetPrevRC();
			if (IntersectRect(&temp, &opRC, &bgRC)) {
				if (opPvRC.bottom <= bgRC.top) {
					(*opIter)->OnGround(bgRC.top);
				}
				else {
					if (opPvRC.top >= bgRC.bottom) {
						(*opIter)->SetPower(0);
					}

					SIZE boxSize = (*opIter)->GetHitBoxSize();
					if (opPvRC.left >= bgRC.right) {
						(*opIter)->SetX(bgRC.right + boxSize.cx / 2);
					}
					else if (opPvRC.right <= bgRC.left) {
						(*opIter)->SetX(bgRC.left - boxSize.cx / 2);
					}
				}
			}
			else {
				RECT tmp = bgRC;
				tmp.top -= 1;
				if (IntersectRect(&temp, &opRC, &tmp)) {
					if (opPvRC.bottom <= tmp.top + 1) {
						(*opIter)->OnGround();
					}
				}
			}
		}
	}

	for (auto blIter = m_bulletList.begin(); blIter != blIterEnd; ++blIter) {
		RECT bulletRC = (*blIter)->GetBulletRC();
		RECT plRC = m_player->GetPlayerRC();

		if ((*blIter)->GetTag() == "Boss") {
			if (OBB(bulletRC, (*blIter)->GetAngle(), plRC, 0)) {
				if (!m_player->IsInvincible()) {
					int dir = m_player->GetPlayerPt().x < (*blIter)->GetBulletPt().x ? 1 : -1;
					m_player->Attacked(50, dir);
					(*blIter)->Die();
					EffectManager::GetInst()->CreateEffectInWorld("Explosion7-0",
						SIZE{ 150, 150 }, (*blIter)->GetBulletPt(), 1);
				}
			}
		}
		else if ((*blIter)->GetTag() == TAGPLAYER) {
			if (m_boss) {
				if (OBB(bulletRC, (*blIter)->GetAngle(), m_boss->GetRC(), 0)) {
					m_boss->HDdown(100);
					(*blIter)->Die();
					EffectManager::GetInst()->CreateEffectInWorld("Explosion7-0",
						SIZE{ 150, 150 }, (*blIter)->GetBulletPt(), 1);
				}
			}
		}
	}
	
	if (m_player and m_portal) {
		RECT pRC = m_player->GetPlayerRC();
		RECT ptRC = m_portal->GetRC();
		RECT temp{ };
		if (IntersectRect(&temp, &ptRC, &pRC)) {
			m_playerInPortal = true;
		}
		else {
			m_playerInPortal = false;
		}
	}

	if (m_player) {
		auto btIterEnd = m_buttonVec.end();
		for (auto iter = m_buttonVec.begin(); iter != btIterEnd; ++iter) {
			RECT btRC = (*iter)->GetButtonRC();
			RECT pRC = m_player->GetPlayerRC();
			RECT temp{ };
			if (IntersectRect(&temp, &pRC, &btRC)) {
				if (!(*iter)->Diying()) {
					(*iter)->Attacked(0, 0, 0, 0);
					CreateBullet(TAGPLAYER, m_player->GetPlayerPt(), MD_TARGET);
				}
			}
		}
	}
#endif
}

void Stage::DeleteDeadObject() {
	if (m_player) {
		if (!m_player->IsAlive() or m_player->Diying()) {
			PlayerDead();
		}
	}

	if (m_boss) {
		if (!m_boss->IsAlive()) {
			SetStage(4);
		}
	}

	list<Monster*>::iterator mIterEnd = m_monsterList.end();
	list<Bullet*>::iterator blIterEnd = m_bulletList.end();
	for (auto iter = m_bulletList.begin(); iter != blIterEnd;) {
		if (!(*iter)->IsAlive()) {
			Bullet* delTarget = (*iter);
			iter = m_bulletList.erase(iter);
			blIterEnd = m_bulletList.end();
			delete delTarget;
		}
		else {
			++iter;
		}
	}

	for (auto iter = m_monsterList.begin(); iter != mIterEnd;) {
		if (!(*iter)->IsAlive()) {
			Monster* delTarget = (*iter);
			iter = m_monsterList.erase(iter);
			mIterEnd = m_monsterList.end();
			delete delTarget;
		}
		else {
			++iter;
		}
	}

	EffectManager::GetInst()->DeleteEndEffect();
}

void Stage::CalCameraPos() {
	if (!m_freeCamera) {
		cameraPos = m_player->GetPlayerPt();
		if (cameraPos.y - clientSize.cy / 2 < 0) {
			cameraPos.y = clientSize.cy / 2;
		}
		else if (cameraPos.y + clientSize.cy / 2 > worldSize.cy) {
			cameraPos.y = worldSize.cy - clientSize.cy / 2;
		}

		if (cameraPos.x - clientSize.cx / 2 < 0) {
			cameraPos.x = clientSize.cx / 2;
		}
		else if (cameraPos.x + clientSize.cx / 2 > worldSize.cx) {
			cameraPos.x = worldSize.cx - clientSize.cx / 2;
		}
	}
}

void Stage::CalCameraPos(const POINT& mousePt)
{
	cameraPos = mousePt;
	if (cameraPos.y - clientSize.cy / 2 < 0) {
		cameraPos.y = clientSize.cy / 2;
	}
	else if (cameraPos.y + clientSize.cy / 2 > worldSize.cy) {
		cameraPos.y = worldSize.cy - clientSize.cy / 2;
	}

	if (cameraPos.x - clientSize.cx / 2 < 0) {
		cameraPos.x = clientSize.cx / 2;
	}
	else if (cameraPos.x + clientSize.cx / 2 > worldSize.cx) {
		cameraPos.x = worldSize.cx - clientSize.cx / 2;
	}
}

void Stage::CameraMove(float deltaTime, int moveDir) {
	float speed = 500.f;
	switch (moveDir) {
	case MD_LEFT:
		cameraPos.x -= static_cast<int>(round(speed * deltaTime));
		if (cameraPos.x - clientSize.cx / 2 < 0) {
			cameraPos.x = clientSize.cx / 2;
		}
		break;

	case MD_RIGHT:
		cameraPos.x += static_cast<int>(round(speed * deltaTime));
		if (cameraPos.x + clientSize.cx / 2 > worldSize.cx) {
			cameraPos.x = worldSize.cx - clientSize.cx / 2;
		}
		break;

	case MD_UP:
		cameraPos.y -= static_cast<int>(round(speed * deltaTime));
		if (cameraPos.y - clientSize.cy / 2 < 0) {
			cameraPos.y = clientSize.cy / 2;
		}
		break;

	case MD_DOWN:
		cameraPos.y += static_cast<int>(round(speed * deltaTime));
		if (cameraPos.y + clientSize.cy / 2 > worldSize.cy) {
			cameraPos.y = worldSize.cy - clientSize.cy / 2;
		}
		break;
	}
}

bool Stage::Save(const string& fileName) {
	SetCurrentDirectory(workingDirNameW.c_str());

	FILE* fp{ };
	fopen_s(&fp, fileName.c_str(), "wb");
	if (!fp) {
		return false;
	}

	FwriteString(m_backGroundFileName, fp);

	list<Monster*>::iterator mIterEnd = m_monsterList.end();
	list<Barigate*>::iterator bIterEnd = m_barigateList.end();

	if (m_player) {
		string tag{ TAGPLAYER };
		FwriteString(tag, fp);
		m_player->Save(fp);
	}

	for (auto iter = m_monsterList.begin(); iter != mIterEnd; ++iter) {
		if ((*iter)) {
			string tag{ (*iter)->GetTag() };
			FwriteString(tag, fp);
			if (!(*iter)->Save(fp)) {
				return false;
			}
		}
	}

	for (auto iter = m_barigateList.begin(); iter != bIterEnd; ++iter) {
		if ((*iter)) {
			string tag{ TAGBARIGATE };
			FwriteString(tag, fp);
			if (!(*iter)->Save(fp)) {
				return false;
			}
		}
	}

	auto btIterEnd = m_buttonVec.end();
	for (auto iter = m_buttonVec.begin(); iter != btIterEnd; ++iter) {
		if ((*iter)) {
			string tag{ TAGBUTTON };
			FwriteString(tag, fp);
			if (!(*iter)->Save(fp)) {
				return false;
			}
		}
	}

	if (m_portal) {
		string tag{ TAGPORTAL };
		FwriteString(tag, fp);
		if (!m_portal->Save(fp)) {
			return false;
		}
	}

	fclose(fp);
	
	return false;
}

bool Stage::Load(const string& fileName) {
	SetCurrentDirectory(workingDirNameW.c_str());

	FILE* fp{ };
	fopen_s(&fp, fileName.c_str(), "rb");
	if (!fp) {
		return false;
	}

	m_backGroundFileName.clear();
	FreadString(m_backGroundFileName, fp);

	while(!feof(fp)) {
		string tag{ };
		FreadString(tag, fp);

		if (tag == TAGMUSHROOM) {
			BadOrange* newObj = new BadOrange;
			newObj->Load(fp);
			m_monsterList.push_back(newObj);
		}
		else if (tag == TAGSLIME) {
			Slime* newObj = new Slime;
			newObj->Load(fp);
			m_monsterList.push_back(newObj);
		}
		else if (tag == TAGPLAYER) {
			if (!m_player) {

				Player* newObj = new Player(true);
				newObj->Load(fp);
				m_player = newObj;

				for (int i = 0; i < 4; i++)
				{
					Player* OtherPlayer = new Player(false);
					m_otherPlayerList.push_back(OtherPlayer);
				}
			}
		}
		else if (tag == TAGBARIGATE) {
			Barigate* newObj = new Barigate;
			newObj->Load(fp);
			m_barigateList.push_back(newObj);
		}
		else if (tag == TAGPORTAL) {
			Portal* newObj = new Portal;
			newObj->Load(fp);
			m_portal = newObj;
		}
		else if (tag == TAGBUTTON) {
			Button* newObj = new Button;
			newObj->Load(fp);
			m_buttonVec.push_back(newObj);
		}
		tag.clear();
	}

	fclose(fp);

	return true;
}

void Stage::AddPlayer(Player* player) {
	if (m_player) {
		delete m_player;
		m_player = nullptr;
	}
	m_player = player;
}

void Stage::AddMonster(Monster* monster) {
	m_monsterList.push_back(monster);
}

void Stage::AddBarigate(Barigate* barigate) {
	m_barigateList.push_back(barigate);
}

void Stage::AddPortal(Portal* portal) {
	if (m_portal) {
		delete m_portal;
	}
	m_portal = portal;
}

void Stage::AddButton(Button* button) {
	m_buttonVec.push_back(button);
}

void Stage::DeleteObjEdit(const POINT& dPt) {
	list<Monster*>::iterator mIterEnd = m_monsterList.end();
	for (auto iter = m_monsterList.begin(); iter != mIterEnd;) {
		RECT rc = (*iter)->GetMonsterRC();
		if (PtInRect(&rc, dPt)) {
			Monster* delTarget = (*iter);
			iter = m_monsterList.erase(iter);
			mIterEnd = m_monsterList.end();
			delete delTarget;
		}
		else {
			++iter;
		}
	}

	list<Barigate*>::iterator bIterEnd = m_barigateList.end();
	for (auto iter = m_barigateList.begin(); iter != bIterEnd;) {
		RECT rc = (*iter)->GetRC();
		if (PtInRect(&rc, dPt)) {
			Barigate* delTarget = (*iter);
			iter = m_barigateList.erase(iter);
			bIterEnd = m_barigateList.end();
			delete delTarget;
		}
		else {
			++iter;
		}
	}

	if (m_player) {
		RECT rc = m_player->GetPlayerRC();
		if (PtInRect(&rc, dPt)) {
			delete m_player;
			m_player = nullptr;
		}
	}

	if (m_portal) {
		RECT rc = m_portal->GetRC();
		if (PtInRect(&rc, dPt)) {
			delete m_portal;
			m_portal = nullptr;
		}
	}

	auto btIterEnd = m_buttonVec.end(); 
	for (auto iter = m_buttonVec.begin(); iter != btIterEnd;) {
		RECT rc = (*iter)->GetButtonRC();
		if (PtInRect(&rc, dPt)) {
			Button* delTarget = (*iter);
			iter = m_buttonVec.erase(iter);
			btIterEnd = m_buttonVec.end();
		}
		else {
			++iter;
		}
	}
}
