//-----------------------------------------------------------------------------
// File: CGameObject.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "Object.h"
#include "Shader.h"
#include "Player.h"
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

	m_xmf4x4World = Matrix4x4::Identity();
	SetPosition(EnemyPosition);

	FallingAcc = 0;
	UfoMissileDelay = 40;
	UfoDead = false;
}


void CGameObject::AnimateUfo(float fTimeElapsed)
{
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

	XMFLOAT3 xmf3LockedObjectPosition = player->GetPosition();
	XMVECTOR xmvLockedObjectPosition = XMLoadFloat3(&xmf3LockedObjectPosition);
	XMVECTOR xmvToLockedObject = xmvLockedObjectPosition - xmvPosition;
	xmvToLockedObject = XMVector3Normalize(xmvToLockedObject);

	XMVECTOR xmvMovingDirection = XMLoadFloat3(&m_xmf3MovingDirection);
	xmvMovingDirection = XMVector3Normalize(XMVectorLerp(xmvMovingDirection, xmvToLockedObject, 10.0 * fTimeElapsed));
	XMStoreFloat3(&m_xmf3MovingDirection, xmvMovingDirection);

	LookTo(m_xmf3MovingDirection, XMFLOAT3(0.0, 1.0, 0.0));
	Move(m_xmf3MovingDirection, 100 * fTimeElapsed);
	moveDistance += fTimeElapsed * 100;

	Rotate(0.0, 0.0, 400 * fTimeElapsed);

	// 일정 거리 이상 이동하면 비활성화 된다
	if (moveDistance > 250)
		ActivateState = false;
}


// shield
void CGameObject::AnimateShield(XMFLOAT3 position, float fTimeElapsed) {
	SetPosition(position);
	Rotate(50 * fTimeElapsed, 50 * fTimeElapsed, 50 * fTimeElapsed);
}


// missile
void CGameObject::AnimateMissile(float fTimeElapsed) {
	Move(m_xmf3MovingDirection, 100 * fTimeElapsed);
	moveDistance += fTimeElapsed * 100;

	Rotate(0.0, 0.0, 400 * fTimeElapsed);

	// 일정 거리 이상 이동하면 비활성화 된다
	if (moveDistance > 250)
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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CUfoObject::CUfoObject()
{
}

CUfoObject::~CUfoObject()
{
}