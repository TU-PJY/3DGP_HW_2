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


	// 쉴드 업데이트
	void AnimateShield(XMFLOAT3 position, float fTimeElapsed);

	// 쉴드 체력 및 쉴드 사용 가능 여부
	float ShieldHP = 1.0;



	// ufo 업데이트
	void AnimateUfo(float fTimeElapsed);

	// ufo 피격 후 재생성
	void RegenUfo();

	// UFO가 움직이는 방향 및 위치
	XMFLOAT3 EnemyPosition{ 1.0f, 1.0f, 1.0f };
	int MoveDirection{};

	// UFO가 플레이어 미사일 피격 시 활성화
	bool UfoDead{};

	// UFO가 피킹된 상태
	bool UfoPickedState{};

	// ufo 피격 시 사용되는 가속값
	float FallingAcc{};

	// ufo 미사일 생성 딜레이
	float UfoMissileDelay{};


	// 미사일 회전 각도
	float MissileRoll{};

	// 플레이어 미사일 추격 대상 객체
	CGameObject* Target{};


	// 미사일 날아가는 방향 지정
	void SetMovingDirection(XMFLOAT3& xmf3MovingDirection);

	// ufo 미사일 업데이트
	void AnimateUfoMissile(float fTimeElapsed, CPlayer* player);

	// 플레이어 미사일 업데이트
	void AnimateMissile(float fTimeElapse);

	// 미사일 위치 이동
	void Move(XMFLOAT3& vDirection, float fSpeed);

	// 미사일이 출력되고 업데이트 되는 여부
	bool ActivateState{};

	// 미사일 날아가는 방향
	XMFLOAT3 m_xmf3MovingDirection = XMFLOAT3(0.0f, 0.0f, 1.0f);

	// 미사일 이동거리
	double MissileMoveDistance{};


	// 시작 매뉴 애니메이션
	void AnimateStartMenu(float fTimeElapsed);


	// 충돌처리 OOBB
	BoundingOrientedBox			m_xmOOBB = BoundingOrientedBox();

	void SetMesh(CMesh* pMesh);
	void SetShader(CShader* pShader);

	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void ReleaseShaderVariables();

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
	void LookTo(XMFLOAT3& xmf3LookTo, XMFLOAT3& xmf3Up);

	void Rotate(float fPitch = 10.0f, float fYaw = 10.0f, float fRoll = 10.0f);
	void Rotate(XMFLOAT3* pxmf3Axis, float fAngle);
	
	// OOBB 업데이트
	void UpdateBoundingBox();

	// 피킹 레이 생성
	void GenerateRayForPicking(XMVECTOR& xmvPickPosition, XMMATRIX& xmmtxView, XMVECTOR& xmvPickRayOrigin, XMVECTOR& xmvPickRayDirection);
	
	// 마우스 피킹
	int PickObjectByRayIntersection(XMVECTOR& xmvPickPosition, XMMATRIX& xmmtxView, float* pfHitDistance);
};

class CUfoObject : public CGameObject
{
public:
	CUfoObject();
	virtual ~CUfoObject();
};
