#ifndef INODE_H
#define INODE_H

#include "math/matrix.h"

// stl
#include <string>

#pragma warning(disable : 4250 )

class IGeometry;

class INode
{
public:
	virtual ~INode() {}
	virtual unsigned int        		GetChildCount() const = 0;
	virtual INode*              		GetChild( unsigned int nIdx ) const = 0;  	
  	virtual void                		Link( INode* pNode ) = 0;
	virtual void						Unlink() = 0;
  	virtual void                		AddChild( INode* pNode ) = 0;
	virtual void						RemoveChild(int index) = 0;
	virtual void						ClearChildren() = 0;
  	virtual void                		Yaw(float fAngle) = 0;
  	virtual void                		Pitch(float fAngle) = 0;
  	virtual void                		Roll(float fAngle) = 0;
  	virtual void                		LocalTranslate(float dx , float dy , float dz) = 0;
	virtual void						WorldTranslate(float dx, float dy, float dz) = 0;
	virtual void                		SetLocalPosition(float x, float y, float z) = 0;
	virtual void                		SetLocalPosition(const CVector& vPos) = 0;
  	virtual void                		SetWorldPosition(float x, float y , float z) = 0;
	virtual void						GetWorldPosition(CVector& vPosition) const = 0;
	virtual void						GetLocalPosition(CVector& vPosition) const = 0;
	virtual void						SetLocalMatrix( const CMatrix& oMat ) = 0;
	virtual void						GetLocalMatrix( CMatrix& oMat ) const = 0;
	virtual const CMatrix&				GetLocalMatrix() const = 0;
	virtual void						GetWorldQuaternion(CQuaternion& oQuaternion) = 0;
	virtual void						SetName(string sName) = 0;
	virtual void						GetName( std::string& sName ) const = 0;
	virtual const string&				GetName() const = 0;
	virtual void						SetID( int nID ) = 0;
	virtual int							GetID() const = 0;
	virtual void						GetWorldMatrix( CMatrix& oMatrix ) = 0;
	virtual void						SetWorldMatrix(const CMatrix& oMat) = 0;
	virtual void						SetQuaternionMode(bool bQuarternion) = 0;
	virtual const CMatrix&				GetWorldMatrix() const = 0;
	virtual void						ConstantLocalTranslate(const CVector& vTranslate) = 0;
	virtual INode*						GetParent() = 0;
	virtual void						SetLocalTMByWorldTM() = 0;
	virtual float						GetGroundHeight(float x, float z) = 0;
	virtual float						GetBoundingSphereDistance(INode* pEntity) = 0;
	virtual float						GetBoundingSphereRadius() const = 0;
	virtual IGeometry*					GetBoundingGeometry() = 0;
	virtual float						GetY() = 0;
	virtual float						GetHeight() = 0;
	virtual void						UpdateTime(float fTime) = 0;
	virtual void						Update() = 0;
	virtual void						GetHierarchyCount(int& nCount) = 0;
	virtual INode*						DuplicateHierarchy() = 0;
	virtual bool						IsCollidable() = 0;
};

#endif // INODE_H