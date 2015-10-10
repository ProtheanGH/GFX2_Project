#include "MoveComponent.h"

#include "Object.h"

// ===== Constructors / Destructors ===== //
MoveComponent::MoveComponent(Object* _parent)
{
	m_ParentObject = _parent;
	m_Wapoints = nullptr;
	m_CurrentTarget = nullptr;
	m_iCurrentWaypoint = 0;
	m_fSpeed = 1.0f;
	m_bPatrolling = false;
}

MoveComponent::~MoveComponent()
{
	// === Clean up all Memory
	delete[] m_Wapoints;
	delete m_CurrentTarget;
}
// ====================================== //

// ===== Interface ===== //
void MoveComponent::Update(float _deltaTime)
{
	// === Update DeltaTime
	DeltaTime = _deltaTime;

	// === Do we need to do anything?
	if (m_CurrentTarget != nullptr) {
		if (MoveTo(m_CurrentTarget))
			SetDestination(nullptr);
	}
	else if (m_bPatrolling) {
		Patrol();
	}
}

void MoveComponent::Patrol(unsigned int _startIndex)
{
	SetDestination(nullptr);
	m_bPatrolling = true;
	m_iCurrentWaypoint = _startIndex;
}

void MoveComponent::StopPatrolling()
{
	m_bPatrolling = false;
}
// ===================== //

// ===== Private Interface ===== //

// - MoveTo
// --- Takes in a Float3 representing the position to move to
// --- Returns true once the positon has been reached, false otherwise
bool MoveComponent::MoveTo(XMFLOAT3* _position)
{
	// === Get the Direction we need to travel in
	XMVECTOR direction = XMVectorSubtract(XMLoadFloat3(_position), XMLoadFloat3(&m_ParentObject->GetPosition()));
	direction = XMVector3Normalize(direction);

	// === Scale it by our Speed
	direction = XMVectorScale(direction, m_fSpeed * DeltaTime);

	// === Move our Object
	XMStoreFloat4x4(&m_ParentObject->WorldMatrix, XMMatrixMultiply(XMMatrixTranslationFromVector(direction), XMLoadFloat4x4(&m_ParentObject->WorldMatrix)));

	// === Have we reached our Destination?
	XMFLOAT3 distance;
	XMStoreFloat3(&distance, XMVector3Length(XMVectorSubtract(XMLoadFloat3(&m_ParentObject->GetPosition()), XMLoadFloat3(_position))));
	if (distance.x < 0.05f)
		return true;
	return false;
}

void MoveComponent::Patrol()
{
	// === Move Towards the Current Waypoint
	if (MoveTo(&m_Wapoints[m_iCurrentWaypoint])) {
		m_iCurrentWaypoint == m_iNumWaypoints - 1 ? m_iCurrentWaypoint = 0 : m_iCurrentWaypoint++;
	}
}
// ============================= //

// ===== Accessors ===== //
void MoveComponent::SetWaypoints(XMFLOAT3* _waypoints, unsigned int _count)
{
	delete[] m_Wapoints;
	m_Wapoints = _waypoints;
	m_iNumWaypoints = _count;
}

void MoveComponent::SetDestination(XMFLOAT3* _position)
{
	delete m_CurrentTarget;
	m_CurrentTarget = _position;
}

void MoveComponent::SetSpeed(float _speed)
{
	m_fSpeed = _speed;
}
// ===================== //
