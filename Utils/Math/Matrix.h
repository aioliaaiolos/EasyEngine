#ifndef MATRIX_H_
#define MATRIX_H_



#include "math.h"
#include "Vector.h"
#include "Utils/IStorage.h"

// stl
#include <vector>

using namespace std;
class CQuaternion;

class  CMatrix : public IPersistantObject
{	
	
public:	
	float				m_00,m_01,m_02,m_03;	
	float				m_10,m_11,m_12,m_13;	
	float				m_20,m_21,m_22,m_23;	
	float				m_30,m_31,m_32,m_33;	


	
						CMatrix();
						CMatrix(	float a00, float a01, float a02, float a03,
									float a10, float a11, float a12, float a13,
									float a20, float a21, float a22, float a23, 
									float a30, float a31, float a32, float a33  );
						CMatrix(const float* params);
						CMatrix(const CVector& v0, const CVector& v1, const CVector& v2, const CVector& v3, bool fillInLine);
						CMatrix(float dVal[][4]);
						CMatrix( float fDiag );
	virtual				~CMatrix();

	void				Fill(	float a00, float a01, float a02, float a03, float a10, float a11, float a12, float a13,
								float a20, float a21, float a22, float a23, float a30, float a31, float a32, float a33 );

	const CMatrix&		operator=(const CMatrix& mat);
	void				operator=(const float* fValues);
	CMatrix				operator+(const CMatrix& mat) const;
	CMatrix				operator -(const CMatrix& mat) const;
	CVector				operator*(const CVector& v) const;
	CMatrix				operator*(const CMatrix& mat) const;
	CMatrix				operator*( float f );
	void				operator+=(const CMatrix& mat);
	void				operator*=(const CMatrix& mat);
	CMatrix				operator/(float d);
	bool				operator==(const CMatrix& mat)const;
		
	void				Get( float* arr ) const;
	void				Get( std::vector< float >& vData ) const;
	void				Set( const float* arr );
	void				Set( const vector< float >& vData );
	void				GetAffinePart( CVector& v );
	void				GetxBase(CVector& v) const;
	void				GetyBase(CVector& v) const;
	void				GetzBase(CVector& v) const;
	void				GetInf(float& nInfx,float& nInfy,float& nInfz) const;
	void				GetSup(float& nSupx,float& nSupy,float& nSupz) const;
	CMatrix				GetTranspose() const;
	void				GetInverse( CMatrix& oMat ) const;
	CMatrix				GetRotation() const;
	CVector				GetPosition() const;
	void				GetPosition(float &x, float &y, float &z) const;
	void				GetPosition(CVector& pos) const;
	CMatrix				GetxRotation();
	CMatrix				GetyRotation();
	CMatrix				GetzRotation();
	CMatrix				GetTranslation();
	float				GetDeterminant()const;
	void				GetQuaternion( CQuaternion& quat ) const;
	//void				GetQuaternion2( CQuaternion& quat ) const;

	bool				IsIdentity() const;
	void				SetIdentity();
	void				SetPosition( float x, float y, float z );
	void				SetAffinePart( const CMatrix& mat );
	void				SetAffinePart( float x, float y, float z );
	void				SetAffinePart( const CVector& v );
	void				AddTranslation( const CVector& v );
	void				AddTranslation( float x, float y, float z );

	static CMatrix		GetxRotation(float Angle);
	static CMatrix		GetyRotation(float Angle);
	static CMatrix		GetzRotation(float Angle);
	static CMatrix		GetTranslation(const CVector& vTranslation);
	static CMatrix		GetTranslation(float x,float y,float z);
	static void			Slerp( const CMatrix& oInitial, const CMatrix& oFinal, float t, CMatrix& oResult );
	static void			LerpAffinePart( const CMatrix& oInitial, const CMatrix& oFinal, float t, CMatrix& oResult );
	static void			GetPassage( const CMatrix& mInitial, const CMatrix& mFinal, CMatrix& mPassage );
	static void			GetAxeAngleRotation( CVector vAxis, float fAngle, CMatrix& mOut );
	void				GetAxeAngleRotation( float ux, float uy, float uz, float fAngle, CMatrix& mOut );
	static void			GetInterpolationMatrix( const CMatrix& oLast, const CMatrix& oNext, CMatrix& oResult, float t );
	//static void		  SlerpInvariantAxis( const CMatrix& oInitial, const CMatrix& oFinal, float t, CMatrix& oResult );

	const IPersistantObject& operator >> (CBinaryFileStorage& store) const;
	IPersistantObject& operator << (const CBinaryFileStorage& store);
	const IPersistantObject& operator >> (CAsciiFileStorage& store) const;
	IPersistantObject& operator << (const CAsciiFileStorage& store);
	const IPersistantObject& operator >> (CStringStorage& store) const;
	IPersistantObject& operator << (const CStringStorage& store);

	void Afficher() const;
};	

class CMatrix2X2
{
public:	
	float				m_00, m_01, m_02;
	float				m_10, m_11, m_12;
	float				m_20, m_21, m_22;
	
	

	CMatrix2X2();
	CMatrix2X2::CMatrix2X2( float a00, float a01, float a02, 
							float a10, float a11, float a12,
							float a20, float a21, float a22	);

	CVector2D		operator*(const CVector2D& v) const;
	CMatrix2X2		operator*(const CMatrix2X2& mat) const;
	void			SetIdentity();
	float			GetDeterminant()const;
	void			GetInverse( CMatrix2X2& oMatrixInv ) const;
	void			AddTranslation( const CVector2D& v );

	static CMatrix2X2	GetRotation(float Angle);
};


#endif //MATRIX_H_