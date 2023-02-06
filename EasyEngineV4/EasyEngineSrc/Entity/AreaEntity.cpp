#include "AreaEntity.h"
#include "IGeometry.h"
#include "MobileEntity.h"
#include "IPhysic.h"

CAreaEntity::CAreaEntity(string areaName, EEInterface& oInterface, IBox& oBox) :
	CBoxEntity(oInterface, oBox),
	m_oInitialMinPoint(oBox.GetMinPoint()),
	m_oInitialDimension(oBox.GetDimension()),
	m_oCurrentMinPoint(m_oInitialMinPoint),
	m_oCurrentDimension(m_oInitialDimension)
{
	m_sEntityID = areaName;
	GetWorldPosition(m_oLastPosition);
	m_pBody->m_fWeight = 0.f;
	m_bIsCollidable = false;
}

void CAreaEntity::Update()
{
	CBoxEntity::Update();
}

float CAreaEntity::GetBoundingSphereRadius() const
{
	return m_oBox.GetBoundingSphereRadius();
}

void CAreaEntity::DrawBoundingBox(bool bDraw)
{
	m_oColor = bDraw ? CVector(1, 1, 1) : CVector(0, 0.5, 1);
}

void CAreaEntity::SetScaleFactor(float x, float y, float z)
{
	m_oCurrentDimension = CVector(m_oCurrentDimension.m_x * x, m_oCurrentDimension.m_y * y, m_oCurrentDimension.m_z * z);
	m_oCurrentMinPoint = -m_oCurrentDimension / 2.f;
	m_oBox.Set(m_oCurrentMinPoint, m_oCurrentDimension);
}

void CAreaEntity::GetScaleFactor(CVector& scale)
{
	scale = CVector(m_oInitialDimension.m_x / m_oCurrentDimension.m_x, m_oInitialDimension.m_y / m_oCurrentDimension.m_y, m_oInitialDimension.m_z / m_oCurrentDimension.m_z, 1.f);
}

void CAreaEntity::ManageGravity()
{
}

void CAreaEntity::UpdateCollision()
{
	float h = 10.f;
	CMatrix backupLocal = m_oLocalMatrix;
	CNode::UpdateWorldMatrix();
	CMatrix oLocalMatrix = m_oLocalMatrix;
	vector<INode*> entities;
	if (!m_bFirstUpdate)
		GetEntitiesCollision(entities);
	else
		m_bFirstUpdate = false;

	CVector localPos;
	oLocalMatrix.GetPosition(localPos);

	CVector first = backupLocal.GetPosition();
	CVector last = localPos;
	CVector firstBottom = first;
	CVector lastBottom = last;
	CVector R = last;
	bool bCollision = false;
	float fMaxHeight = -999999.f;
	for (int i = 0; i < entities.size(); i++) {
		INode* pEntity = entities[i];
		pEntity->GetBoundingGeometry()->SetTM(pEntity->GetLocalMatrix());
		IGeometry* firstBox = GetBoundingGeometry()->Duplicate();
		firstBox->SetTM(backupLocal);
		IGeometry* lastBox = GetBoundingGeometry();
		lastBox->SetTM(oLocalMatrix);
		IGeometry::TFace collisionFace = IGeometry::eNone;
		collisionFace = pEntity->GetBoundingGeometry()->GetReactionYAlignedBox(*firstBox, *lastBox, R);
		delete firstBox;
		if (collisionFace != IBox::eNone) {
			lastBottom = R;
			last = R;
			bCollision = true;
			if (collisionFace == IBox::eYPositive) {
				last.m_y += h / 2.f;
				if (fMaxHeight < last.m_y)
					fMaxHeight = last.m_y;
				else
					last.m_y = fMaxHeight;
				m_pBody->m_oSpeed.m_y = 0;
			}
		}
		else {
			bCollision = true;
		}
	}
	// Ground collision
	const float margin = 10.f;
	CVector nextPosition = m_oLocalMatrix * CVector(0, 0, 100);
	float fGroundHeight = m_pParent->GetGroundHeight(localPos.m_x, localPos.m_z) + margin;
	float fGroundHeightNext = m_pParent->GetGroundHeight(nextPosition.m_x, nextPosition.m_z) + margin;
	float fEntityY = last.m_y - h / 2.f;
	if (fEntityY <= fGroundHeight + m_pPhysic->GetEpsilonError()) {
		m_pBody->m_oSpeed.m_x = 0;
		m_pBody->m_oSpeed.m_y = 0;
		m_pBody->m_oSpeed.m_z = 0;
		float newY = fGroundHeight + h / 2.f;
		last.m_y = newY;
	}
	SetLocalPosition(last);

	// Still into parent ?	
	if (!TestWorldCollision(m_pParent)) {
		CEntity* pEntity = dynamic_cast<CEntity*>(m_pParent->GetParent());
		if (pEntity)
			LinkAndUpdateMatrices(pEntity);
	}

	if (bCollision && m_pfnCollisionCallback) {
		m_pfnCollisionCallback(this, entities);
	}
}

