//------------------------------------------------------- ----------------------
// File: Object.h
//-----------------------------------------------------------------------------

#pragma once
#include <string>
#include "Mesh.h"
#include "Camera.h"

class CShader;

class CGameObject
{
public:
	CGameObject(XMFLOAT3 Position);
	CGameObject();
	virtual ~CGameObject();

public:
	XMFLOAT4X4						m_xmf4x4World;
	CMesh* m_pMesh = NULL;
	CShader* m_pShader = NULL;
	XMFLOAT3						m_xmf3Color = XMFLOAT3(1.0f, 1.0f, 1.0f);

	// UFO가 움직이는 방향 및 위치
	XMFLOAT3 EnemyPosition{ 1.0f, 1.0f, 1.0f };
	int MoveDirection{};

	// UFO가 플레이어 미사일 피격 시 활성화
	bool UfoDead{};

	// ufo 피격 시 사용되는 가속값
	float acc{};

	// ufo 미사일 생성 딜레이
	float UfoMissileDelay{};

	// ufo 피격 후 재생성
	void RegenUfo();

	// ufo 미사일 업데이트
	void AnimateUfoMissile(float fTimeElapsed);

	// 미사일이 출력되고 업데이트 되는 여부
	bool activateState{};

	// 미사일 날아가는 방향
	XMFLOAT3 m_xmf3MovingDirection = XMFLOAT3(0.0f, 0.0f, 1.0f);

	// 미사일 날아가는 방향 지정
	void SetMovingDirection(XMFLOAT3& xmf3MovingDirection);

	// 미사일 위치 이동
	void Move(XMFLOAT3& vDirection, float fSpeed);

	// 미사일 이동거리
	double moveDistance{};

	// 충돌처리 OOBB
	BoundingOrientedBox			m_xmOOBB = BoundingOrientedBox();

	void SetMesh(CMesh* pMesh);
	void SetShader(CShader* pShader);

	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void ReleaseShaderVariables();

	virtual void AnimateUfo(float fTimeElapsed);
	void AnimateShield(XMFLOAT3 position, float fTimeElapsed);
	void CGameObject::AnimateMissile(float fTimeElapsed);

	virtual void OnPrepareRender() { }
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL);

	virtual void ReleaseUploadBuffers();

	XMFLOAT3 GetPosition();
	XMFLOAT3 GetLook();
	XMFLOAT3 GetUp();
	XMFLOAT3 GetRight();

	void SetPosition(float x, float y, float z);
	void SetPosition(XMFLOAT3 xmf3Position);

	void SetColor(XMFLOAT3 xmf3Color) { m_xmf3Color = xmf3Color; }

	void MoveStrafe(float fDistance = 1.0f);
	void MoveUp(float fDistance = 1.0f);
	void MoveForward(float fDistance = 1.0f);
	void LookAt(XMFLOAT3& xmf3LookAt, XMFLOAT3& xmf3Up);

	void Rotate(float fPitch = 10.0f, float fYaw = 10.0f, float fRoll = 10.0f);
	void Rotate(XMFLOAT3* pxmf3Axis, float fAngle);

	void UpdateBoundingBox();
};

class CUfoObject : public CGameObject
{
public:
	CUfoObject();
	virtual ~CUfoObject();
};
