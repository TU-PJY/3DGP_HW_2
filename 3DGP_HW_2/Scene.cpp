//-----------------------------------------------------------------------------
// File: CScene.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "Scene.h"
#include "Player.h"

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
#endif

	m_nObjects = 4;
	m_ppObjects = new CGameObject * [m_nObjects];

	CPseudoLightingShader* pShader = new CPseudoLightingShader();
	pShader->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature);
	pShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);

	m_ppObjects[0] = new CGameObject(XMFLOAT3(-10.0f, 3.0f, 20.0f));
	m_ppObjects[0]->SetMesh(pUfoMesh);
	m_ppObjects[0]->SetShader(pShader);
	m_ppObjects[0]->SetColor(XMFLOAT3(0.7f, 0.0f, 0.0f));
	m_ppObjects[0]->Rotate(0.0, 180.0f, 0.0);

	m_ppObjects[1] = new CGameObject(XMFLOAT3(10.0f, -3.0f, 20.0f));
	m_ppObjects[1]->SetMesh(pUfoMesh);
	m_ppObjects[1]->SetShader(pShader);
	m_ppObjects[1]->SetColor(XMFLOAT3(0.7f, 0.0f, 0.0f));
	m_ppObjects[1]->Rotate(0.0, 180.0f, 0.0);

	m_ppObjects[2] = new CGameObject(XMFLOAT3(1.0f, -6.0f, 20.0f));
	m_ppObjects[2]->SetMesh(pUfoMesh);
	m_ppObjects[2]->SetShader(pShader);
	m_ppObjects[2]->SetColor(XMFLOAT3(0.7f, 0.0f, 0.0f));
	m_ppObjects[2]->Rotate(0.0, 180.0f, 0.0);

	m_ppObjects[3] = new CGameObject(XMFLOAT3(-1.0f, 6.0f, 20.0f));
	m_ppObjects[3]->SetMesh(pUfoMesh);
	m_ppObjects[3]->SetShader(pShader);
	m_ppObjects[3]->SetColor(XMFLOAT3(0.7f, 0.0f, 0.0f));
	m_ppObjects[3]->Rotate(0.0, 180.0f, 0.0);

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

	if (m_ppObjects)
	{
		for (int j = 0; j < m_nObjects; j++) if (m_ppObjects[j]) delete m_ppObjects[j];
		delete[] m_ppObjects;
	}
}

void CScene::ReleaseUploadBuffers()
{
	if (m_ppObjects)
	{
		for (int j = 0; j < m_nObjects; j++) if (m_ppObjects[j]) m_ppObjects[j]->ReleaseUploadBuffers();
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
	// m_pShield
	m_pShield->AnimateShield(m_pPlayer->m_xmf3Position, fTimeElapsed);

	// activateState가 true일 때만 업데이트 한다
	for (int i = 0; i < m_nMissiles; ++i) {
		if (m_pMissile[i]->activateState) {
			m_pMissile[i]->AnimateMissile(fTimeElapsed);
			m_pMissile[i]->UpdateBoundingBox();
		}
	}

	for (int i = 0; i < m_nObjects; i++) {
		m_ppObjects[i]->Animate(fTimeElapsed);
		m_ppObjects[i]->UpdateBoundingBox();
	}


	CheckObjectByBulletCollisions();
}

void CScene::PrepareRender(ID3D12GraphicsCommandList* pd3dCommandList) {
	pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);
}

void CScene::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera) {
	pCamera->SetViewportsAndScissorRects(pd3dCommandList);
	pCamera->UpdateShaderVariables(pd3dCommandList);

	for (int i = 0; i < m_nObjects; i++)
		if (m_ppObjects[i]) m_ppObjects[i]->Render(pd3dCommandList, pCamera);

	// activateState가 true일 때만 랜더링한다
	for (int i = 0; i < m_nMissiles; ++i) {
		if (m_pMissile[i]->activateState)
			m_pMissile[i]->Render(pd3dCommandList, pCamera);
	}

	if (m_pShield && m_pPlayer->shieldState)
		m_pShield->Render(pd3dCommandList, pCamera);
}

//  플레이어 미사일 생성
void CScene::CreateMissile() {
	for (int i = 0; i < m_nMissiles; ++i) {
		if (!m_pMissile[i]->activateState) {
			XMFLOAT3 xmf3Up = m_pPlayer->GetUp();
			XMFLOAT3 xmf3Direction = m_pPlayer->GetLook();
			XMFLOAT3 xmf3Position = m_pPlayer->GetPosition();
			XMFLOAT3 xmf3FirePosition = Vector3::Add(xmf3Position, Vector3::ScalarProduct(xmf3Direction, -4.0f, false));

			m_pMissile[i]->m_xmf4x4World = Matrix4x4::Identity();

			m_pMissile[i]->LookAt(xmf3Direction, xmf3Up);
			m_pMissile[i]->SetPosition(xmf3FirePosition);
			m_pMissile[i]->SetMovingDirection(xmf3Direction);

			m_pMissile[i]->moveDistance = 0;
			m_pMissile[i]->activateState = true;
			break;
		}
	}
}

// 미사일 - ufo 충돌 처리
void CScene::CheckObjectByBulletCollisions() {
	for (int i = 0; i < m_nMissiles; ++i) {
		for (int j = 0; j < m_nObjects; ++j) {
			if (m_pMissile[i]->activateState && m_ppObjects[j]->m_xmOOBB.Intersects(m_pMissile[i]->m_xmOOBB)) {
				m_pMissile[i]->activateState = false;
			}
		}
	}

}