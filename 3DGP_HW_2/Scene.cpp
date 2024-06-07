//-----------------------------------------------------------------------------
// File: CScene.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "Scene.h"
#include "Player.h"
#include "Camera.h"
#include <cmath>

CScene::CScene()
{
}

CScene::~CScene()
{
}

void CScene::InputPlayer(CPlayer* player) {
	m_pPlayer = player;
}

//#define _WITH_TEXT_MODEL_FILE
#define _WITH_BINARY_MODEL_FILE

void CScene::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	m_pd3dGraphicsRootSignature = CreateGraphicsRootSignature(pd3dDevice);

#ifdef _WITH_TEXT_MODEL_FILE
	CMesh* pUfoMesh = new CMesh(pd3dDevice, pd3dCommandList, "Models/UFO.txt", true);
	CMesh* pFlyerMesh = new CMesh(pd3dDevice, pd3dCommandList, "Models/FlyerPlayership.txt", true);
#endif
#ifdef _WITH_BINARY_MODEL_FILE
	CMesh* pUfoMesh = new CMesh(pd3dDevice, pd3dCommandList, "Models//UFO.txt", true);
	CMesh* pShieldMesh = new CMesh(pd3dDevice, pd3dCommandList, "Models//shield.txt", true);
	CMesh* pMissileMesh = new CMesh(pd3dDevice, pd3dCommandList, "Models//missile.txt", true);
	CMesh* pStartMenuMesh = new CMesh(pd3dDevice, pd3dCommandList, "Models//start_menu.txt", true);
#endif

	m_nUfos = 4;
	m_pUfo = new CGameObject * [m_nUfos];

	CPseudoLightingShader* pShader = new CPseudoLightingShader();
	pShader->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature);
	pShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);

	m_pUfo[0] = new CGameObject(XMFLOAT3(-10.0f, 3.0f, 20.0f));
	m_pUfo[0]->SetMesh(pUfoMesh);
	m_pUfo[0]->SetShader(pShader);
	m_pUfo[0]->SetColor(XMFLOAT3(0.7f, 0.0f, 0.0f));

	m_pUfo[1] = new CGameObject(XMFLOAT3(10.0f, -3.0f, 20.0f));
	m_pUfo[1]->SetMesh(pUfoMesh);
	m_pUfo[1]->SetShader(pShader);
	m_pUfo[1]->SetColor(XMFLOAT3(0.7f, 0.0f, 0.0f));

	m_pUfo[2] = new CGameObject(XMFLOAT3(1.0f, -6.0f, 20.0f));
	m_pUfo[2]->SetMesh(pUfoMesh);
	m_pUfo[2]->SetShader(pShader);
	m_pUfo[2]->SetColor(XMFLOAT3(0.7f, 0.0f, 0.0f));

	m_pUfo[3] = new CGameObject(XMFLOAT3(-1.0f, 6.0f, 20.0f));
	m_pUfo[3]->SetMesh(pUfoMesh);
	m_pUfo[3]->SetShader(pShader);
	m_pUfo[3]->SetColor(XMFLOAT3(0.7f, 0.0f, 0.0f));

	// m_pShield
	m_pShield = new CGameObject(XMFLOAT3(0.0, 0.0, 0.0));
	m_pShield->SetMesh(pShieldMesh);
	m_pShield->SetShader(pShader);
	m_pShield->SetColor(XMFLOAT3(0.0, 1.0, 0.0));

	// m_pMissile
	m_nMissiles = 100;
	m_pMissile = new CGameObject * [m_nMissiles];

	for (int i = 0; i < m_nMissiles; ++i) {
		m_pMissile[i] = new CGameObject(XMFLOAT3(0.0, 0.0, 0.0));
		m_pMissile[i]->SetMesh(pMissileMesh);
		m_pMissile[i]->SetShader(pShader);
		m_pMissile[i]->SetColor(XMFLOAT3(0.3, 0.3, 0.3));
	}

	// m_pUfoMissile
	m_nUfoMissiles = 50;
	m_pUfoMissile = new CGameObject * [m_nUfoMissiles];

	for (int i = 0; i < m_nUfoMissiles; ++i) {
		m_pUfoMissile[i] = new CGameObject(XMFLOAT3(0.0, 0.0, 0.0));
		m_pUfoMissile[i]->SetMesh(pMissileMesh);
		m_pUfoMissile[i]->SetShader(pShader);
		m_pUfoMissile[i]->SetColor(XMFLOAT3(1.0, 0.0, 0.0));
	}

	// m_pStartMenu
	m_pStartMenu = new CGameObject(XMFLOAT3(0.0, 0.0, 0.0));
	m_pStartMenu->SetMesh(pStartMenuMesh);
	m_pStartMenu->SetShader(pShader);
	m_pStartMenu->SetColor(XMFLOAT3(0.0, 0.0, 0.0));
	m_pStartMenu->SetPosition(XMFLOAT3(0.0, 3.0, -4.0));
	m_pStartMenu->Rotate(0.0, 180.0, 0.0);
}

ID3D12RootSignature* CScene::CreateGraphicsRootSignature(ID3D12Device* pd3dDevice)
{
	ID3D12RootSignature* pd3dGraphicsRootSignature = NULL;

	D3D12_ROOT_PARAMETER pd3dRootParameters[3];
	pd3dRootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	pd3dRootParameters[0].Constants.Num32BitValues = 4; //Time, ElapsedTime, xCursor, yCursor
	pd3dRootParameters[0].Constants.ShaderRegister = 0; //Time
	pd3dRootParameters[0].Constants.RegisterSpace = 0;
	pd3dRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	pd3dRootParameters[1].Constants.Num32BitValues = 19; //16 + 3
	pd3dRootParameters[1].Constants.ShaderRegister = 1; //World
	pd3dRootParameters[1].Constants.RegisterSpace = 0;
	pd3dRootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	pd3dRootParameters[2].Constants.Num32BitValues = 35; //16 + 16 + 3
	pd3dRootParameters[2].Constants.ShaderRegister = 2; //Camera
	pd3dRootParameters[2].Constants.RegisterSpace = 0;
	pd3dRootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	D3D12_ROOT_SIGNATURE_DESC d3dRootSignatureDesc;
	::ZeroMemory(&d3dRootSignatureDesc, sizeof(D3D12_ROOT_SIGNATURE_DESC));
	d3dRootSignatureDesc.NumParameters = _countof(pd3dRootParameters);
	d3dRootSignatureDesc.pParameters = pd3dRootParameters;
	d3dRootSignatureDesc.NumStaticSamplers = 0;
	d3dRootSignatureDesc.pStaticSamplers = NULL;
	d3dRootSignatureDesc.Flags = d3dRootSignatureFlags;

	ID3DBlob* pd3dSignatureBlob = NULL;
	ID3DBlob* pd3dErrorBlob = NULL;
	D3D12SerializeRootSignature(&d3dRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pd3dSignatureBlob, &pd3dErrorBlob);
	pd3dDevice->CreateRootSignature(0, pd3dSignatureBlob->GetBufferPointer(), pd3dSignatureBlob->GetBufferSize(), __uuidof(ID3D12RootSignature), (void**)&pd3dGraphicsRootSignature);
	if (pd3dSignatureBlob) pd3dSignatureBlob->Release();
	if (pd3dErrorBlob) pd3dErrorBlob->Release();

	return(pd3dGraphicsRootSignature);
}

void CScene::ReleaseObjects()
{
	if (m_pd3dGraphicsRootSignature) m_pd3dGraphicsRootSignature->Release();

	if (m_pUfo)
	{
		for (int j = 0; j < m_nUfos; j++) if (m_pUfo[j]) delete m_pUfo[j];
		delete[] m_pUfo;
	}
}

void CScene::ReleaseUploadBuffers()
{
	if (m_pUfo)
	{
		for (int j = 0; j < m_nUfos; j++) if (m_pUfo[j]) m_pUfo[j]->ReleaseUploadBuffers();
	}
}

bool CScene::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	return(false);
}

bool CScene::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	return(false);
}

bool CScene::ProcessInput()
{
	return(false);
}

void CScene::AnimateObjects(float fTimeElapsed){
	if (GameRunningState) {
		// m_pShield
		m_pShield->AnimateShield(m_pPlayer->m_xmf3Position, fTimeElapsed);

		// m_pMissile
		// activateState가 true일 때만 업데이트 한다
		for (int i = 0; i < m_nMissiles; ++i) {
			if (m_pMissile[i]->ActivateState) {
				m_pMissile[i]->AnimateMissile(fTimeElapsed);
				m_pMissile[i]->UpdateBoundingBox();
			}
		}

		// m_pUfo
		for (int i = 0; i < m_nUfos; i++) {
			// ufo는 플레이어를 바라본다
			if (!m_pUfo[i]->UfoDead)
				m_pUfo[i]->LookAt(m_pPlayer->GetPosition(), XMFLOAT3(0.0f, 1.0f, 0.0f));

			m_pUfo[i]->AnimateUfo(fTimeElapsed);

			// 미사일 생성 딜레이가 0 이하가 되면 새로운 미사일을 생성한다
			if (m_pUfo[i]->UfoMissileDelay <= 0)
				CreateUfoMissile(i);

			m_pUfo[i]->UpdateBoundingBox();
		}

		// m_pUfoMissile
		for (int i = 0; i < m_nUfoMissiles; ++i) {
			if (m_pUfoMissile[i]->ActivateState) {
				m_pUfoMissile[i]->AnimateUfoMissile(fTimeElapsed, m_pPlayer);
				m_pUfoMissile[i]->UpdateBoundingBox();
			}
		}

		PlayerMissileToUfoCollision();
		UfoMissileToPlayerCollision();
	}
}


void CScene::PrepareRender(ID3D12GraphicsCommandList* pd3dCommandList) {
	pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);
}


void CScene::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera) {
	pCamera->SetViewportsAndScissorRects(pd3dCommandList);
	pCamera->UpdateShaderVariables(pd3dCommandList);

	if (GameRunningState) {
		// m_pShield
		if (m_pShield && m_pPlayer->ShieldState)
			m_pShield->Render(pd3dCommandList, pCamera);

		// m_pMissile
		// activateState가 true일 때만 랜더링한다
		for (int i = 0; i < m_nMissiles; ++i) {
			if (m_pMissile[i]->ActivateState)
				m_pMissile[i]->Render(pd3dCommandList, pCamera);
		}

		// m_pUfo
		for (int i = 0; i < m_nUfos; i++)
			if (m_pUfo[i]) m_pUfo[i]->Render(pd3dCommandList, pCamera);

		// m_pUfoMissile
		for (int i = 0; i < m_nUfoMissiles; ++i) {
			if (m_pUfoMissile[i]->ActivateState)
				m_pUfoMissile[i]->Render(pd3dCommandList, pCamera);
		}
	}

	else
		m_pStartMenu->Render(pd3dCommandList, pCamera);
}


//  플레이어 미사일 생성
void CScene::CreateMissile() {
	for (int i = 0; i < m_nMissiles; ++i) {
		if (!m_pMissile[i]->ActivateState) {
			XMFLOAT3 xmf3Up = m_pPlayer->GetUp();
			XMFLOAT3 xmf3Direction = m_pPlayer->GetLook();
			XMFLOAT3 xmf3Position = m_pPlayer->GetPosition();
			XMFLOAT3 xmf3FirePosition = Vector3::Add(xmf3Position, Vector3::ScalarProduct(xmf3Direction, -4.0f, false));

			m_pMissile[i]->m_xmf4x4World = Matrix4x4::Identity();

			m_pMissile[i]->LookAt(xmf3Direction, xmf3Up);
			m_pMissile[i]->SetPosition(xmf3FirePosition);
			m_pMissile[i]->SetMovingDirection(xmf3Direction);

			m_pMissile[i]->moveDistance = 0;
			m_pMissile[i]->ActivateState = true;
			break;
		}
	}
}


// ufo 미사일 생성
void CScene::CreateUfoMissile(int i) {
	for (int j = 0; j < m_nUfoMissiles; ++j) {
		if (!m_pUfoMissile[j]->ActivateState) {
			XMFLOAT3 xmf3Up = m_pUfo[i]->GetUp();
			XMFLOAT3 xmf3Direction = m_pUfo[i]->GetLook();
			XMFLOAT3 xmf3Position = m_pUfo[i]->GetPosition();
			XMFLOAT3 xmf3FirePosition = Vector3::Add(xmf3Position, Vector3::ScalarProduct(xmf3Direction, -4.0f, false));

			m_pUfoMissile[j]->m_xmf4x4World = Matrix4x4::Identity();

			m_pUfoMissile[j]->LookAt(xmf3Direction, xmf3Up);
			m_pUfoMissile[j]->SetPosition(xmf3FirePosition);
			m_pUfoMissile[j]->SetMovingDirection(xmf3Direction);

			m_pUfoMissile[j]->moveDistance = 0;
			m_pUfoMissile[j]->ActivateState = true;
			break;
		}
	}

	m_pUfo[i]->UfoMissileDelay = 40;
}


// 플레이어 미사일 - ufo 충돌 처리
void CScene::PlayerMissileToUfoCollision() {
	for (int i = 0; i < m_nMissiles; ++i) {
		for (int j = 0; j < m_nUfos; ++j) {
			if (m_pMissile[i]->ActivateState && !m_pUfo[j]->UfoDead && m_pUfo[j]->m_xmOOBB.Intersects(m_pMissile[i]->m_xmOOBB)) {
				m_pMissile[i]->ActivateState = false;
				m_pUfo[j]->FallingAcc = 20.0;
				m_pUfo[j]->UfoDead = true;
			}
		}
	}
}

// ufo 미사일 - 플레이어 충돌 처리
void CScene::UfoMissileToPlayerCollision() {
	for (int i = 0; i < m_nUfoMissiles; ++i) {
		if (m_pUfoMissile[i]->ActivateState && m_pPlayer->m_xmOOBB.Intersects(m_pUfoMissile[i]->m_xmOOBB)) {

			// 쉴드 활성화 상태일 때는 쉴드가 대미지를 받는다
			if (m_pPlayer->ShieldState) {
				m_pShield->ShieldHP -= 0.025;

				// 체력이 떨어질 수록 빨간색으로 변한다
				m_pShield->SetColor(XMFLOAT3(1.0 - m_pShield->ShieldHP, m_pShield->ShieldHP, 0.0));

				// 쉴드의 체력이 완전히 떨어지면 더 이상 쉴드를 활성화 할 수 없다
				if (m_pShield->ShieldHP <= 0) {
					m_pPlayer->ShieldState = false;
					m_pShield->ShieldHP = 0;
				}
			}

			// 그렇지 않다면 플레이어가 대미지를 받는다
			else {
				m_pPlayer->PlayerHP -= 0.025;

				// 체력이 떨어질 수록 빨간색으로 변한다
				m_pPlayer->SetColor(XMFLOAT3(1.0 - m_pPlayer->PlayerHP, 0.0, m_pPlayer->PlayerHP));

				// 플레이어의 체력이 완전히 떨어지면 게임이 초기화 된다
				if (m_pPlayer->PlayerHP <= 0)
					ResetGame();
			}

			m_pUfoMissile[i]->ActivateState = false;
		}
	}
}

// 게임 초기화
void CScene::ResetGame() {
	m_pPlayer->PlayerHP = 1.0;
	m_pPlayer->SetColor(XMFLOAT3(1.0 - m_pPlayer->PlayerHP, 0.0, m_pPlayer->PlayerHP));

	m_pShield->ShieldHP = 1.0;
	m_pShield->SetColor(XMFLOAT3(1.0 - m_pShield->ShieldHP, m_pShield->ShieldHP, 0.0));
	m_pPlayer->ShieldState = true;

	m_pPlayer->SetPosition(XMFLOAT3(0.0, 0.0, 0.0));

	// 플레이어 미사일 모두 비활성화
	for (int i = 0; i < m_nMissiles; ++i)
		m_pMissile[i]->ActivateState = false;

	// ufo 미사일 모두 비활성화
	for (int i = 0; i < m_nUfoMissiles; ++i)
		m_pUfoMissile[i]->ActivateState = false;

	// ufo 초기화
	for (int i = 0; i < m_nUfos; ++i) {
		m_pUfo[i]->m_xmf4x4World = Matrix4x4::Identity();
		m_pUfo[i]->UfoDead = false;
		m_pUfo[i]->UfoMissileDelay = 40;
	}

	// 초기 위치로 이동
	m_pUfo[0]->SetPosition(XMFLOAT3(-10.0f, 3.0f, 20.0f));
	m_pUfo[1]->SetPosition(XMFLOAT3(10.0f, -3.0f, 20.0f));
	m_pUfo[2]->SetPosition(XMFLOAT3(1.0f, -6.0f, 20.0f));
	m_pUfo[3]->SetPosition(XMFLOAT3(-1.0f, 6.0f, 20.0f));

	// 게임 동작 상태 비활성화
	GameRunningState = false;
}