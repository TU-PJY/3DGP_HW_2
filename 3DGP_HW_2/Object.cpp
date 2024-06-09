//-----------------------------------------------------------------------------
// File: CGameObject.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "Object.h"
#include "Shader.h"
#include "Player.h"
#include "Scene.h"
#include <cmath>
#include <random>

CGameObject::CGameObject(XMFLOAT3 Position)
{
	m_xmf4x4World = Matrix4x4::Identity();
	EnemyPosition = Position;

	UfoMissileDelay = 40;

	// 음수이면 오른쪽, 양수이면 왼쪽으로 움직이도록 방향 설정
	if (Position.x < 0) MoveDirection = 1;
	else MoveDirection = -1;
}

CGameObject::CGameObject() {
	m_xmf4x4World = Matrix4x4::Identity();
}

CGameObject::~CGameObject()
{
	if (m_pMesh) m_pMesh->Release();
	if (m_pShader) m_pShader->Release();
}

void CGameObject::SetMesh(CMesh* pMesh)
{
	if (m_pMesh) m_pMesh->Release();
	m_pMesh = pMesh;
	if (m_pMesh) m_pMesh->AddRef();
}

void CGameObject::SetShader(CShader* pShader)
{
	if (m_pShader) m_pShader->Release();
	m_pShader = pShader;
	if (m_pShader) m_pShader->AddRef();
}


// Ufo 피격 후 재생성
void CGameObject::RegenUfo() {
	std::random_device rd;
	std::uniform_real_distribution urdX{ -29.0, 29.0 };
	std::uniform_real_distribution urdY{ -20.0, 20.0 };
	std::uniform_real_distribution urdZ{ -6.0, 6.0 };

	EnemyPosition.x = urdX(rd);
	EnemyPosition.y = urdZ(rd);
	EnemyPosition.z = urdY(rd);

	if (EnemyPosition.x < 0) MoveDirection = 1;
	else MoveDirection = -1;

	m_xmf4x4World = Matrix4x4::Identity();
	SetPosition(EnemyPosition);

	SetColor(XMFLOAT3(0.7, 0.0, 0.0));
	UfoMissileDelay = 40;
	UfoPickedState = false;
	UfoDead = false;
}


void CGameObject::AnimateUfo(float fTimeElapsed) {
	// ufo 미사일 피격 전
	if (!UfoDead) {
		EnemyPosition.x += fTimeElapsed * MoveDirection * 10;

		if (EnemyPosition.x > 30.0 || EnemyPosition.x < -30.0)
			MoveDirection *= -1;

		SetPosition(EnemyPosition);

		if(UfoMissileDelay > 0)
			UfoMissileDelay -= fTimeElapsed * 25;
	}

	// ufo 미사일 피격 후
	else {
		EnemyPosition.y += 2 * FallingAcc * fTimeElapsed;
		FallingAcc -= 40 * fTimeElapsed;

		SetPosition(EnemyPosition);
		Rotate(800 * fTimeElapsed, 800 * fTimeElapsed, 800 * fTimeElapsed);

		if (EnemyPosition.y < -80)
			RegenUfo();
	}
}


// ufo missile
void CGameObject::AnimateUfoMissile(float fTimeElapsed, CPlayer* player) {
	XMFLOAT3 xmf3Position = GetPosition();
	XMVECTOR xmvPosition = XMLoadFloat3(&xmf3Position);

	XMFLOAT3 xmf3targetPosition = player->GetPosition();
	XMVECTOR xmvtargetPosition = XMLoadFloat3(&xmf3targetPosition);
	XMVECTOR xmvToTargetObject = xmvtargetPosition - xmvPosition;
	xmvToTargetObject = XMVector3Normalize(xmvToTargetObject);

	XMVECTOR xmvMovingDirection = XMLoadFloat3(&m_xmf3MovingDirection);
	xmvMovingDirection = XMVector3Normalize(XMVectorLerp(xmvMovingDirection, xmvToTargetObject, 12.0 * fTimeElapsed));
	XMStoreFloat3(&m_xmf3MovingDirection, xmvMovingDirection);

	LookTo(m_xmf3MovingDirection, XMFLOAT3(0.0, 1.0, 0.0));
	Move(m_xmf3MovingDirection, 100 * fTimeElapsed);

	Rotate(0.0, 0.0, ObjectRoll);

	MissileMoveDistance += fTimeElapsed * 100;
	ObjectRoll += 400 * fTimeElapsed;

	// 일정 거리 이상 이동하면 비활성화 된다
	if (MissileMoveDistance > 250)
		ActivateState = false;
}


// shield
void CGameObject::AnimateShield(XMFLOAT3 position, float fTimeElapsed) {
	Rotate(50 * fTimeElapsed, 50 * fTimeElapsed, 50 * fTimeElapsed);
	SetPosition(position);
}


// missile
// 피킹된 ufo가 존재하면 해당 ufo를 향해 날아간다
void CGameObject::AnimateMissile(float fTimeElapsed) {
	if (Target && !Target->UfoDead) {
		XMFLOAT3 xmf3Position = GetPosition();
		XMVECTOR xmvPosition = XMLoadFloat3(&xmf3Position);

		XMFLOAT3 xmf3TargetPosition = Target->GetPosition();
		XMVECTOR xmvTargetPosition = XMLoadFloat3(&xmf3TargetPosition);
		XMVECTOR xmvToTargetObject = xmvTargetPosition - xmvPosition;
		xmvToTargetObject = XMVector3Normalize(xmvToTargetObject);

		XMVECTOR xmvMovingDirection = XMLoadFloat3(&m_xmf3MovingDirection);
		xmvMovingDirection = XMVector3Normalize(XMVectorLerp(xmvMovingDirection, xmvToTargetObject, 12.0 * fTimeElapsed));
		XMStoreFloat3(&m_xmf3MovingDirection, xmvMovingDirection);

		LookTo(m_xmf3MovingDirection, XMFLOAT3(0.0, 1.0, 0.0));
		Move(m_xmf3MovingDirection, 100 * fTimeElapsed);

		ObjectRoll += fTimeElapsed * 400;
		MissileMoveDistance += fTimeElapsed * 100;

		// 미사일이 회전하면서 날아간다
		Rotate(0.0, 0.0, ObjectRoll);
	}

	else {
		Move(m_xmf3MovingDirection, 100 * fTimeElapsed);

		MissileMoveDistance += fTimeElapsed * 100;

		Rotate(0.0, 0.0, 400 * fTimeElapsed);
	}

	// 일정 거리 이상 이동하면 비활성화 된다
	if (MissileMoveDistance > 250)
		ActivateState = false;
}

void CGameObject::AnimateStartMenu(float fTimeElapsed) {
	Rotate(0.0, fTimeElapsed * 100.0, 0.0);
}

void CGameObject::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
}

void CGameObject::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	XMFLOAT4X4 xmf4x4World;
	XMStoreFloat4x4(&xmf4x4World, XMMatrixTranspose(XMLoadFloat4x4(&m_xmf4x4World)));
	pd3dCommandList->SetGraphicsRoot32BitConstants(1, 16, &xmf4x4World, 0);

	pd3dCommandList->SetGraphicsRoot32BitConstants(1, 3, &m_xmf3Color, 16);
}

void CGameObject::ReleaseShaderVariables()
{
}

void CGameObject::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	OnPrepareRender();

	if (m_pShader) 
		m_pShader->Render(pd3dCommandList, pCamera);

	UpdateShaderVariables(pd3dCommandList);

	if (m_pMesh) 
		m_pMesh->Render(pd3dCommandList);
}

void CGameObject::ReleaseUploadBuffers()
{
	if (m_pMesh) m_pMesh->ReleaseUploadBuffers();
}

void CGameObject::Move(XMFLOAT3& vDirection, float fSpeed) {
	SetPosition(m_xmf4x4World._41 + vDirection.x * fSpeed, m_xmf4x4World._42 + vDirection.y * fSpeed, m_xmf4x4World._43 + vDirection.z * fSpeed);
}

void CGameObject::LookAt(XMFLOAT3& xmf3LookAt, XMFLOAT3& xmf3Up)
{
	XMFLOAT4X4 xmf4x4View = Matrix4x4::LookAtLH(GetPosition(), xmf3LookAt, xmf3Up);
	m_xmf4x4World._11 = xmf4x4View._11; m_xmf4x4World._12 = xmf4x4View._21; m_xmf4x4World._13 = xmf4x4View._31;
	m_xmf4x4World._21 = xmf4x4View._12; m_xmf4x4World._22 = xmf4x4View._22; m_xmf4x4World._23 = xmf4x4View._32;
	m_xmf4x4World._31 = xmf4x4View._13; m_xmf4x4World._32 = xmf4x4View._23; m_xmf4x4World._33 = xmf4x4View._33;
}

void CGameObject::LookTo(XMFLOAT3& xmf3LookTo, XMFLOAT3& xmf3Up)
{
	XMFLOAT4X4 xmf4x4View = Matrix4x4::LookToLH(GetPosition(), xmf3LookTo, xmf3Up);
	m_xmf4x4World._11 = xmf4x4View._11; m_xmf4x4World._12 = xmf4x4View._21; m_xmf4x4World._13 = xmf4x4View._31;
	m_xmf4x4World._21 = xmf4x4View._12; m_xmf4x4World._22 = xmf4x4View._22; m_xmf4x4World._23 = xmf4x4View._32;
	m_xmf4x4World._31 = xmf4x4View._13; m_xmf4x4World._32 = xmf4x4View._23; m_xmf4x4World._33 = xmf4x4View._33;
}

void CGameObject::SetMovingDirection(XMFLOAT3& xmf3MovingDirection) {
	m_xmf3MovingDirection = Vector3::Normalize(xmf3MovingDirection);
}

void CGameObject::SetPosition(float x, float y, float z)
{
	m_xmf4x4World._41 = x;
	m_xmf4x4World._42 = y;
	m_xmf4x4World._43 = z;
}

void CGameObject::SetPosition(XMFLOAT3 xmf3Position)
{
	SetPosition(xmf3Position.x, xmf3Position.y, xmf3Position.z);
}

void CGameObject::UpdateBoundingBox(){
	if (m_pMesh){
		m_pMesh->m_xmOOBB.Transform(m_xmOOBB, XMLoadFloat4x4(&m_xmf4x4World));
		XMStoreFloat4(&m_xmOOBB.Orientation, XMQuaternionNormalize(XMLoadFloat4(&m_xmOOBB.Orientation)));
	}
}

XMFLOAT3 CGameObject::GetPosition()
{
	return(XMFLOAT3(m_xmf4x4World._41, m_xmf4x4World._42, m_xmf4x4World._43));
}

XMFLOAT3 CGameObject::GetLook()
{
	return(Vector3::Normalize(XMFLOAT3(m_xmf4x4World._31, m_xmf4x4World._32, m_xmf4x4World._33)));
}

XMFLOAT3 CGameObject::GetUp()
{
	return(Vector3::Normalize(XMFLOAT3(m_xmf4x4World._21, m_xmf4x4World._22, m_xmf4x4World._23)));
}

XMFLOAT3 CGameObject::GetRight()
{
	return(Vector3::Normalize(XMFLOAT3(m_xmf4x4World._11, m_xmf4x4World._12, m_xmf4x4World._13)));
}

void CGameObject::MoveStrafe(float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Right = GetRight();
	xmf3Position = Vector3::Add(xmf3Position, xmf3Right, fDistance);
	CGameObject::SetPosition(xmf3Position);
}

void CGameObject::MoveUp(float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Up = GetUp();
	xmf3Position = Vector3::Add(xmf3Position, xmf3Up, fDistance);
	CGameObject::SetPosition(xmf3Position);
}

void CGameObject::MoveForward(float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Look = GetLook();
	xmf3Position = Vector3::Add(xmf3Position, xmf3Look, fDistance);
	CGameObject::SetPosition(xmf3Position);
}

void CGameObject::Rotate(float fPitch, float fYaw, float fRoll)
{
	XMMATRIX mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(fPitch), XMConvertToRadians(fYaw), XMConvertToRadians(fRoll));
	m_xmf4x4World = Matrix4x4::Multiply(mtxRotate, m_xmf4x4World);
}

void CGameObject::Rotate(XMFLOAT3* pxmf3Axis, float fAngle)
{
	XMMATRIX mtxRotate = XMMatrixRotationAxis(XMLoadFloat3(pxmf3Axis), XMConvertToRadians(fAngle));
	m_xmf4x4World = Matrix4x4::Multiply(mtxRotate, m_xmf4x4World);
}


void CGameObject::GenerateRayForPicking(XMVECTOR& xmvPickPosition, XMMATRIX& xmmtxView, XMVECTOR& xmvPickRayOrigin, XMVECTOR& xmvPickRayDirection)
{
	XMMATRIX xmmtxToModel = XMMatrixInverse(NULL, XMLoadFloat4x4(&m_xmf4x4World) * xmmtxView);

	XMFLOAT3 xmf3CameraOrigin(0.0f, 0.0f, 0.0f);
	xmvPickRayOrigin = XMVector3TransformCoord(XMLoadFloat3(&xmf3CameraOrigin), xmmtxToModel);
	xmvPickRayDirection = XMVector3TransformCoord(xmvPickPosition, xmmtxToModel);
	xmvPickRayDirection = XMVector3Normalize(xmvPickRayDirection - xmvPickRayOrigin);
}


int CGameObject::PickObjectByRayIntersection(XMVECTOR& xmvPickPosition, XMMATRIX& xmmtxView, float* pfHitDistance)
{
	int nIntersected = 0;
	if (m_pMesh)
	{
		XMVECTOR xmvPickRayOrigin, xmvPickRayDirection;
		GenerateRayForPicking(xmvPickPosition, xmmtxView, xmvPickRayOrigin, xmvPickRayDirection);
		nIntersected = m_pMesh->CheckRayIntersection(xmvPickRayOrigin, xmvPickRayDirection, pfHitDistance);
	}
	return(nIntersected);
}


// ufo 피킹
void CScene::PickObjectPointedByCursor(int xClient, int yClient, CCamera* pCamera) {
	XMFLOAT3 xmf3PickPosition;
	xmf3PickPosition.x = (((2.0f * xClient) / (float)pCamera->m_d3dViewport.Width) - 1) / pCamera->m_xmf4x4Projection._11;
	xmf3PickPosition.y = -(((2.0f * yClient) / (float)pCamera->m_d3dViewport.Height) - 1) / pCamera->m_xmf4x4Projection._22;
	xmf3PickPosition.z = 1.0f;

	XMVECTOR xmvPickPosition = XMLoadFloat3(&xmf3PickPosition);
	XMMATRIX xmmtxView = XMLoadFloat4x4(&pCamera->m_xmf4x4View);

	int nIntersected = 0;
	float fNearestHitDistance = FLT_MAX;

	// 모든 ufo 피킹 상태 초기화
	for (int i = 0; i < m_nUfos; i++) {
		m_pUfo[i]->SetColor(XMFLOAT3(0.7, 0.0, 0.0));
		m_pUfo[i]->UfoPickedState = false;
	}

	// 피킹이 감지된 ufo 객체는 초록색으로 색이 바뀐다
	for (int i = 0; i < m_nUfos; i++){
		float fHitDistance = FLT_MAX;
		nIntersected = m_pUfo[i]->PickObjectByRayIntersection(xmvPickPosition, xmmtxView, &fHitDistance);

		if ((nIntersected > 0) && (fHitDistance < fNearestHitDistance)) {
			if (!m_pUfo[i]->UfoDead) {
				m_pUfo[i]->SetColor(XMFLOAT3(0.0, 0.8, 0.0));
				m_pUfo[i]->UfoPickedState = true;
			}
			break;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CUfoObject::CUfoObject()
{
}

CUfoObject::~CUfoObject()
{
}