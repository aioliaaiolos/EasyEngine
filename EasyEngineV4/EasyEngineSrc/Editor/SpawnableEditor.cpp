#include "SpawnableEditor.h"
#include "Interface.h"
#include "IInputManager.h"
#include "IRenderer.h"
#include "IEntity.h"
#include "ICamera.h"
#include "IGeometry.h"
#include "ICollisionManager.h"
#include "IConsole.h"
#include "EditorManager.h"

CSpawnableEditor::CSpawnableEditor(EEInterface& oInterface, ICameraManager::TCameraType cameraType) :
CEditor(oInterface, cameraType),
m_oEventDispatcher(static_cast<IEventDispatcher&>(*oInterface.GetPlugin("EventDispatcher"))),
m_pEditingEntity(nullptr),
m_pScene(nullptr),
m_bDisplayPickingRaySelected(false),
m_bDisplayPickingRayMouseMove(false),
m_bDisplayPickingIntersectPlane(false),
m_pQuadEntity(nullptr),
m_pDebugSphere(nullptr),
m_eLastKeyEvent(IEventDispatcher::TKeyEvent::T_KEYUP),
m_eEditorMode(TEditorMode::eNone),
m_eEditingMode(eXForm)
{	
	m_oEventDispatcher.AbonneToMouseEvent(this, OnMouseEventCallback);
}

void CSpawnableEditor::HandleEditorManagerCreation(IEditorManager* pEditorManager)
{
	m_pEditorManager = static_cast<CEditorManager*>(pEditorManager);
}

void CSpawnableEditor::OnMouseEventCallback(CPlugin* plugin, IEventDispatcher::TMouseEvent e, int x, int y)
{
	CSpawnableEditor* pEditor = dynamic_cast<CSpawnableEditor*>(plugin);
	if (pEditor->m_bEditionMode) {
		if (e == IEventDispatcher::TMouseEvent::T_LBUTTONDOWN)
			pEditor->OnLeftMouseDown(x, y);
		else if (e == IEventDispatcher::TMouseEvent::T_RBUTTONDOWN)
			pEditor->m_oInputManager.SetEditionMode(false);
		else if (e == IEventDispatcher::TMouseEvent::T_RBUTTONUP)
			pEditor->m_oInputManager.SetEditionMode(true);
		else if ( (e == IEventDispatcher::TMouseEvent::T_MOVE) && (pEditor->m_eEditorMode == TEditorMode::eAdding) )
			pEditor->OnMouseMove(x, y);
	}
}

void CSpawnableEditor::OnKeyEventCallback(CPlugin* plugin, IEventDispatcher::TKeyEvent e, int key)
{
	CSpawnableEditor* pEditor = dynamic_cast<CSpawnableEditor*>(plugin);
	if (e == IEventDispatcher::T_KEYUP) {
		if (key == VK_SPACE) {
			pEditor->m_eEditingMode = (pEditor->m_eEditingMode == TEditingType::eScale) ? TEditingType::eXForm : TEditingType::eScale;
			return;
		}
	}
	if (e == IEventDispatcher::T_KEYDOWN) {
		if (!pEditor->m_oConsole.IsOpen()) {
			if (pEditor->m_pEditingEntity) {
				if (key == VK_DELETE) {
					pEditor->OnEntityRemoved(pEditor->m_pEditingEntity);
					pEditor->m_pEditingEntity->Unlink();
					pEditor->m_pEditingEntity = nullptr;
				}
				else if (pEditor->m_eEditingMode == TEditingType::eXForm && pEditor->m_pEditingEntity) {
					float yawStep = (pEditor->m_oInputManager.GetKeyState(VK_CONTROL) == IInputManager::KEY_STATE::PRESSED) ? 1.f : 10.f;
					float translateValue = 10.f;
					
					if (key == VK_LEFT) {
						if (pEditor->m_oInputManager.GetKeyState(VK_SHIFT) == IInputManager::KEY_STATE::PRESSED)
							pEditor->m_pEditingEntity->LocalTranslate(translateValue, 0, 0);
						else
							pEditor->m_pEditingEntity->Yaw(-yawStep);
					}
					else if (key == VK_RIGHT) {
						if (pEditor->m_oInputManager.GetKeyState(VK_SHIFT) == IInputManager::KEY_STATE::PRESSED)
							pEditor->m_pEditingEntity->LocalTranslate(-translateValue, 0, 0);
						else
							pEditor->m_pEditingEntity->Yaw(yawStep);
					}
					else if (key == VK_UP) {
						if (pEditor->m_oInputManager.GetKeyState(VK_SHIFT) == IInputManager::KEY_STATE::PRESSED)
							pEditor->m_pEditingEntity->LocalTranslate(0, 0, translateValue);
						else
							pEditor->m_pEditingEntity->LocalTranslate(0, translateValue, 0);
					}
					else if (key == VK_DOWN) {
						if (pEditor->m_oInputManager.GetKeyState(VK_SHIFT) == IInputManager::KEY_STATE::PRESSED)
							pEditor->m_pEditingEntity->LocalTranslate(0, 0, -translateValue);
						else
							pEditor->m_pEditingEntity->LocalTranslate(0, -translateValue, 0);
					}
				}
				else if (pEditor->m_eEditingMode == TEditingType::eScale && pEditor->m_pEditingEntity) {
					CVector scaleFactor;
					pEditor->m_pEditingEntity->GetScaleFactor(scaleFactor);
					float s = 1.01f;
					if (key == VK_LEFT) {
						pEditor->m_pEditingEntity->SetScaleFactor(s, 1, 1);
					}
					else if (key == VK_RIGHT) {
						pEditor->m_pEditingEntity->SetScaleFactor(1.f / s, 1.f, 1.f);
					}
					else if (key == VK_UP) {
						if (pEditor->m_oInputManager.GetKeyState(VK_SHIFT) == IInputManager::KEY_STATE::PRESSED)
							pEditor->m_pEditingEntity->SetScaleFactor(1.f, 1.f, s);
						else
							pEditor->m_pEditingEntity->SetScaleFactor(1.f, s, 1.f);
					}
					else if (key == VK_DOWN) {
						if (pEditor->m_oInputManager.GetKeyState(VK_SHIFT) == IInputManager::KEY_STATE::PRESSED)
							pEditor->m_pEditingEntity->SetScaleFactor(1.f, 1.f, 1.f / s);
						else
							pEditor->m_pEditingEntity->SetScaleFactor(1.f, 1.f / s, 1.f);
					}
				}
			}
		}
	}
}

void CSpawnableEditor::OnMouseMove(int x, int y)
{
	if (m_pEditingEntity) {
		CVector intersect;
		GetRayPlanIntersection(x, y, GetPlanHeight(), intersect);
		m_pEditingEntity->SetLocalPosition(intersect);
		if (m_bDisplayPickingRayMouseMove) {
			CVector camPos, ray_wor;
			RayCast(x, y, camPos, ray_wor);
			CVector farPoint = camPos + ray_wor * 50000.f;
			farPoint.m_w = 1.f;
			DisplayPickingRay(camPos, farPoint);
		}
	}
}

void CSpawnableEditor::OnLeftMouseDown(int x, int y)
{
	if (m_pEditingEntity) {
		m_pEditingEntity->SetWeight(1.f);
		m_pScene->UpdateMapEntities();
		OnEntityAdded();
		m_pEditingEntity->DrawBoundingBox(false);
		m_pEditingEntity = nullptr;
	}
	else {
		CVector intersect;
		SelectEntity(x, y);
		if (m_pEditingEntity) {
			m_pEditingEntity->DrawBoundingBox(true);
			m_eEditorMode = m_oInputManager.GetKeyState(VK_SHIFT) == IInputManager::KEY_STATE::PRESSED ? TEditorMode::eEditing : TEditorMode::eAdding;
			if (m_eEditorMode == TEditorMode::eAdding) {
				CVector pos;
				m_pEditingEntity->GetWorldPosition(pos);
				m_pEditingEntity->Link(m_pScene);
				m_pEditingEntity->SetLocalPosition(pos.m_x, GetPlanHeight(), pos.m_z);
				m_pEditingEntity->Update();
			}
			m_pEditingEntity->SetWeight(0.f);
			OnEntitySelected();
		}
	}
}

void CSpawnableEditor::GetRayPlanIntersection(int x, int y, float h, CVector& intersect)
{
	CVector camPos, ray_wor;
	RayCast(x, y, camPos, ray_wor);

#ifdef DEBUG_TEST_PLANE
	IEntity* pLineEntity = m_oEntityManager.CreateLineEntity(camPos, ray_wor);
	pLineEntity->Link(m_pScene);
#endif // DEBUG_TEST_PLANE

	IMesh* pMesh = static_cast<IMesh*>(m_pScene->GetRessource());
	const CVector& d = pMesh->GetBBox()->GetDimension();
	IQuad* quad = m_oGeometryManager.CreateQuad(d.m_x, d.m_z);
	CMatrix quadTM;
	quadTM.SetPosition(0, h, 0);
	quad->SetTM(quadTM);

	CVector B = camPos + ray_wor * 50000.f;
	B.m_w = 1.f;
	quad->GetLineIntersection(camPos, B, intersect);
	delete quad;

	if (m_bDisplayPickingIntersectPlane) {
		if (!m_pDebugSphere) {
			m_pDebugSphere = m_oEntityManager.CreateSphere(50.f);
			m_pDebugSphere->Link(m_pScene);
		}
		m_pDebugSphere->SetLocalPosition(intersect);

		if (!m_pQuadEntity) {
			m_pQuadEntity = m_oEntityManager.CreateQuad(d.m_x, d.m_z);
			m_pQuadEntity->Link(m_pScene);
		}
		m_pQuadEntity->SetLocalPosition(0.f, h, 0.f);
	}
	else {
		if (m_pQuadEntity) {
			m_pQuadEntity->Unlink();
			delete m_pQuadEntity;
			m_pQuadEntity = nullptr;
		}
	}
}

void CSpawnableEditor::RayCast(int x, int y, CVector& p1, CVector& ray)
{
	CMatrix w, p;
	m_oCameraManager.GetActiveCamera()->GetWorldMatrix(w);
	m_oRenderer.GetProjectionMatrix(p);
	unsigned int width, height;
	m_oRenderer.GetResolution(width, height);
	m_oGeometryManager.RayCast(x, y, w, p, width, height, p1, ray);
}

void CSpawnableEditor::DisplayPickingRay(const CVector& camPos, const CVector& farPoint)
{
	IEntity* pLineEntity = m_oEntityManager.CreateLineEntity(camPos, farPoint);
	pLineEntity->Link(m_pScene);
}

void CSpawnableEditor::SelectEntity(int x, int y)
{
	CVector camPos, ray_wor;
	RayCast(x, y, camPos, ray_wor);
	CVector farPoint = camPos + ray_wor * 50000.f;
	farPoint.m_w = 1.f;

	if (m_bDisplayPickingRaySelected) {
		DisplayPickingRay(camPos, farPoint);
	}

	IEntity* pSelectedEntity = NULL;
	vector<IEntity*> entities;
	CollectSelectableEntity(entities);
	for (int i = 0; i < entities.size(); i++) {
		IEntity* pEntity = entities[i];
		CVector pos;
		pEntity->GetWorldPosition(pos);
		if (m_oGeometryManager.IsIntersect(camPos, farPoint, pos, pEntity->GetBoundingSphereRadius())) {
			float lastDistanceToCam = 999999999.f;
			CVector lastPos, currentPos;
			if (pSelectedEntity) {
				pSelectedEntity->GetWorldPosition(lastPos);
				lastDistanceToCam = (lastPos - camPos).Norm();
			}
			pEntity->GetWorldPosition(currentPos);
			float newDistanceToCam = (currentPos - camPos).Norm();
			if (newDistanceToCam < lastDistanceToCam) {
				pSelectedEntity = pEntity;
			}
		}
	}
	if (pSelectedEntity) {
		if (m_pEditingEntity)
			m_pEditingEntity->DrawBoundingBox(false);
		pSelectedEntity->DrawBoundingBox(true);
		m_pEditingEntity = pSelectedEntity;
		DisplayLocalRepere();
	}
	else if (m_pEditingEntity) {
		m_pEditingEntity->DrawBoundingBox(false);
	}
}

void CSpawnableEditor::DisplayLocalRepere()
{
	return;
	CVector position;
	m_pEditingEntity->GetWorldPosition(position);
	IEntity* x = m_oEntityManager.CreateCylinder(500, 10000);
}

void CSpawnableEditor::EnableDisplayPickingRaySelected(bool enable)
{
	m_bDisplayPickingRaySelected = enable;
}

void CSpawnableEditor::EnableDisplayPickingRayMouseMove(bool enable)
{
	m_bDisplayPickingRayMouseMove = enable;
}

void CSpawnableEditor::EnableDisplayPickingIntersectPlane(bool enable)
{
	m_bDisplayPickingIntersectPlane = enable;
}

void CSpawnableEditor::SetEditionMode(bool bEnable)
{
	if(bEnable)
		m_oEventDispatcher.AbonneToKeyEvent(this, OnKeyEventCallback);
	else
		m_oEventDispatcher.DesabonneToKeyEvent(this, OnKeyEventCallback);
}

bool CSpawnableEditor::IsEnabled()
{
	return m_bEditionMode;
}

void CSpawnableEditor::InitSpawnedEntity()
{
	m_pEditingEntity->Link(m_pScene);
	m_pEditingEntity->SetLocalPosition(0, GetPlanHeight(), 0);
	m_pEditingEntity->SetWeight(0.f);
	m_pEditingEntity->Update();
}

void CSpawnableEditor::InitCamera()
{
	m_oCameraManager.SetActiveCamera(m_pEditorCamera);
	IBox* pBBox = dynamic_cast<IBox*>(m_pScene->GetBoundingGeometry());
	if (pBBox) {
		CMatrix m;
		m_pEditorCamera->SetLocalMatrix(m);
		m_pEditorCamera->SetSpeed(1.f);
		m_pEditorCamera->Move(0, -60.f, 0, 0, 0, pBBox->GetDimension().m_x / 8.f);
	}
}