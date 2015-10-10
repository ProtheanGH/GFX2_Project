#pragma once
#include <DirectXMath.h>

using namespace DirectX;

class Object;

class MoveComponent
{
private:
	Object* m_ParentObject;
	XMFLOAT3* m_Wapoints;
	XMFLOAT3* m_CurrentTarget;
	unsigned int m_iNumWaypoints;
	unsigned int m_iCurrentWaypoint;
	float m_fSpeed;
	bool m_bPatrolling;

	float DeltaTime;

	// ===== Private Interface
	bool MoveTo(XMFLOAT3* _position);
	void Patrol();

public:
	// ===== Constuctors / Destructors
	MoveComponent(Object* _parent);
	~MoveComponent();

	// ===== Interface
	void Update(float _deltaTime);
	void Patrol(unsigned int _startIndex);
	void StopPatrolling();

	// ===== Accessors
	void SetWaypoints(XMFLOAT3* _waypoints, unsigned int _count);
	void SetDestination(XMFLOAT3* _position);
	void SetSpeed(float _speed);

	// ===== Mutators
	float GetSpeed() { return m_fSpeed; }
};

