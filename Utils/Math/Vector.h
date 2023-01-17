#ifndef VECTOR_H_
#define VECTOR_H_

class CMatrix;

#include "Utils/IStorage.h"

class  CVector : public IPersistantObject
{

public:		

	float m_x;
	float m_y;
	float m_z;
	float m_w;

	CVector();
	CVector(float x, float y, float z , float w);
	CVector(float x, float y, float z );
	CVector(float TabVal[4]);
	CVector(const CVector& v);
	~CVector();

	CVector operator *(float d) const;
	float operator *(const CVector& v) const;
	CVector operator *(const CMatrix& mat) const;		
	CVector operator+(const CVector& v) const;
	CVector operator-(const CVector& v) const;
	CVector operator/(const float d) const;
	CVector	operator^( const CVector& v );
	float& operator[]( int i );
	bool operator==( const CVector& v ) const;
	bool operator!=( const CVector& v ) const;
	CVector operator-() const;
	CVector& operator +=( const CVector& v );
	CVector& operator -=( const CVector& v );

	float	Norm();
	void	Normalize();
	void	Fill( float x, float y, float z, float w );	

	const IPersistantObject& operator >> (CBinaryFileStorage& store) const;
	IPersistantObject& operator << (CBinaryFileStorage& store);
	const IPersistantObject& operator >> (CAsciiFileStorage& store) const;
	IPersistantObject& operator << (CAsciiFileStorage& store);
	const IPersistantObject& operator >> (CStringStorage& store) const;
	IPersistantObject& operator << (CStringStorage& store);


	static void		Lerp( const CVector& v1, const CVector& v2, float t, CVector& vOut );
	static float	GetMinx(const vector< CVector >& vPoints);
	static float	GetMiny(const vector< CVector >& vPoints);
	static float	GetMinz(const vector< CVector >& vPoints);
	static float	GetMaxx(const vector< CVector >& vPoints);
	static float	GetMaxy(const vector< CVector >& vPoints);
	static float	GetMaxz(const vector< CVector >& vPoints);

	static float	GetDistance(const CVector& p1, const CVector& p2);

	static int		GetMinxIndex(const vector< CVector >& vPoints);
	static int		GetMinyIndex(const vector< CVector >& vPoints);
	static int		GetMinzIndex(const vector< CVector >& vPoints);
	static int		GetMaxxIndex(const vector< CVector >& vPoints);
	static int		GetMaxyIndex(const vector< CVector >& vPoints);
	static int		GetMaxzIndex(const vector< CVector >& vPoints);
};

class CVector2D : public IPersistantObject
{
public:
	float m_x;
	float m_y;
	float m_w;

	CVector2D();
	CVector2D( float x, float y );

	CVector2D	operator-(const CVector2D& v) const;
	CVector		operator^( const CVector2D& v ) const;
	float		operator*( const CVector2D& v ) const;
	CVector2D	operator*( float d ) const;
	CVector2D	operator+( const CVector2D& v ) const;
	void		Normalize();
	float		Norm() const;

	const IPersistantObject& operator >> (CBinaryFileStorage& store) const;
	IPersistantObject& operator << (CBinaryFileStorage& store);
	const IPersistantObject& operator >> (CAsciiFileStorage& store) const;
	IPersistantObject& operator << (CAsciiFileStorage& store);
	const IPersistantObject& operator >> (CStringStorage& store) const;
	IPersistantObject& operator << (CStringStorage& store);

};

#endif //VECTOR_H