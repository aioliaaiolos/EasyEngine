#ifndef NODE_H
#define NODE_H


// stl
#include <vector>
#include <string>

// Engine
#include "math/matrix.h"
#include "math/Quaternion.h"
#include "INode.h"

using namespace std;

class IGeometry;

class CNode : virtual public INode
{

protected:
	INode*                     	m_pParent;
	vector< INode* >      		m_vChild;
	CMatrix						m_oLocalMatrix;
	CMatrix						m_oWorldMatrix;
	CQuaternion					m_oLocalQuaternion;
	CQuaternion					m_oWorldQuaternion;
	string						m_sName;
	int							m_nID;
	int							m_nParentID;
	bool						m_bQuaternionMode;
	CVector						m_vConstantLocalRotate;
	CVector						m_vConstantLocalTranslate;
	bool						m_bUpdateConstantLocalTranslate;
	bool						m_bUpdateConstantLocalRotate;
	bool						m_bIsCollidable;

	void						UpdateWorldMatrix();
	void						UpdateChildren();

public:
	                    		CNode();
	virtual             		~CNode();
	unsigned int        		GetChildCount() const;
	INode*              		GetChild( unsigned int nIdx ) const;
  	virtual void	    		Update();
  	virtual void                Link( INode* pNode );
	virtual void				Unlink();
  	void                		AddChild(INode* pNode);
	void						RemoveChild(int index);
	void						ClearChildren() override;
  	void                		SetParent( INode* pNode );
  	virtual void                Yaw(float fAngle);
  	virtual void                Pitch(float fAngle);
  	virtual void                Roll(float fAngle);
  	virtual void                LocalTranslate(float dx , float dy , float dz);
	virtual void				WorldTranslate( float dx , float dy , float dz );
	virtual void				WorldTranslate( const CVector& vTranslate );
  	void                		SetLocalPosition(float x, float y , float z);
	void                		SetLocalPosition( const CVector& vPos );
	void                		SetWorldPosition(float x, float y, float z);
	void                		SetWorldPosition(const CVector& vPos);
	virtual void				GetLocalPosition(CVector& vPosition) const;
	virtual void				GetWorldPosition( CVector& vPosition ) const;
	virtual void				SetLocalMatrix( const CMatrix& oMat );
	virtual void				SetWorldMatrix( const CMatrix& oMat );
	virtual void				GetLocalQuaternion( CQuaternion& oQuaternion );
	virtual void				SetLocalQuaternion( const CQuaternion& oQuaternion );
	virtual void				GetLocalMatrix( CMatrix& oMat ) const;
	virtual const CMatrix&		GetLocalMatrix() const;
	virtual void				SetName(string sName);
	virtual void				GetName( std::string& sName ) const;
	virtual const string&		GetName() const;
	virtual INode*				DuplicateHierarchy();
	void						SetID( int nID );
	int							GetID() const;
	void						GetWorldMatrix( CMatrix& oMatrix );
	const CMatrix&				GetWorldMatrix() const;
	void						GetWorldQuaternion( CQuaternion& oQuaternion );
	void						GetHierarchyCount( int& nCount );
	void						SetQuaternionMode( bool bQuarternion );
	INode*						GetParent();
	virtual void				SetLocalTMByWorldTM();
	void						ClearChilds();
	virtual void				ConstantLocalRotate( const CVector& vRotation );
	virtual void				ConstantLocalTranslate( const CVector& vTranslate );
	virtual float				GetDistance(INode* pNode);
	float						GetGroundHeight(float x, float z) override;
	float						GetX();
	float						GetY();
	float						GetZ();
	bool						TestCollision(INode* pNode) override;
	float						GetBoundingSphereDistance(INode* pEntity) override;
	IGeometry*					GetBoundingGeometry() override;
	float						GetHeight() override;
	float						GetBoundingSphereRadius() const override;
	void						UpdateTime(float fTime) override;
	bool						IsCollidable() override;
};

#endif  //NODE_H