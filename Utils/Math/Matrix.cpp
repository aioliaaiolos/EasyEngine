#define _USE_MATH_DEFINES
#include <math.h>
#include "matrix.h"
#include "Quaternion.h"

// Windows
#include <windows.h>

// stl
#include <exception>
#include <sstream>

using namespace std;

//-----------------------------------------------------------------------------------------------------
//											Constructor
//-----------------------------------------------------------------------------------------------------
CMatrix::CMatrix()
{
	this->SetIdentity();
}

//-----------------------------------------------------------------------------------------------------
//											Destructor
//-----------------------------------------------------------------------------------------------------
CMatrix::~CMatrix()
{

}


//-----------------------------------------------------------------------------------------------------
//											Transtypage constructor
//-----------------------------------------------------------------------------------------------------
CMatrix::CMatrix(float dVal[][4])
{
	m_00 = dVal[0][0]	;	m_01 = dVal[0][1]	;	m_02 = dVal[0][2]	;	m_03 = dVal[0][3]	;	
	m_10 = dVal[1][0]	;	m_11 = dVal[1][1]	;	m_12 = dVal[1][2]	;	m_13 = dVal[1][3]	;	
	m_20 = dVal[2][0]	;	m_21 = dVal[2][1]	;	m_22 = dVal[2][2]	;	m_23 = dVal[2][3]	;	
	m_30 = dVal[3][0]	;	m_31 = dVal[3][1]	;	m_32 = dVal[3][2]	;	m_33 = dVal[3][3]	;	

}


//-----------------------------------------------------------------------------------------------------
//											Transtypage constructor
//-----------------------------------------------------------------------------------------------------
CMatrix::CMatrix(	float a11, float a12, float a13, float a14,
						float a21, float a22, float a23, float a24,
						float a31, float a32, float a33, float a34, 
						float a41, float a42, float a43, float a44  )
{
	m_00 = a11;
	m_01 = a12;
	m_02 = a13;
	m_03 = a14;

	m_10 = a21;
	m_11 = a22;
	m_12 = a23;
	m_13 = a24;

	m_20 = a31;
	m_21 = a32;
	m_22 = a33;
	m_23 = a34;;

	m_30 = a41;
	m_31 = a42;
	m_32 = a43;
	m_33 = a44;
}


//-----------------------------------------------------------------------------------------------------
//											Transtypage constructor
//-----------------------------------------------------------------------------------------------------
CMatrix::CMatrix(const float* params)
{
	m_00 = params[0];
	m_01 = params[1];
	m_02 = params[2];
	m_03 = params[3];

	m_10 = params[4];
	m_11 = params[5];
	m_12 = params[6];
	m_13 = params[7];

	m_20 = params[8];
	m_21 = params[9];
	m_22 = params[10];
	m_23 = params[11];

	m_30 = params[12];
	m_31 = params[13];
	m_32 = params[14];
	m_33 = params[15];		
}


//-----------------------------------------------------------------------------------------------------
//											Transtypage constructor
//-----------------------------------------------------------------------------------------------------
CMatrix::CMatrix(const CVector& v0,const CVector& v1,const CVector& v2,const CVector& v3, bool fillInLine)
{
	if (fillInLine) {
		m_00 = v0.m_x;
		m_01 = v0.m_y;
		m_02 = v0.m_z;
		m_03 = v0.m_w;

		m_10 = v1.m_x;
		m_11 = v1.m_y;
		m_12 = v1.m_z;
		m_13 = v1.m_w;

		m_20 = v2.m_x;
		m_21 = v2.m_y;
		m_22 = v2.m_z;
		m_23 = v2.m_w;

		m_30 = v3.m_x;
		m_31 = v3.m_y;
		m_32 = v3.m_z;
		m_33 = v3.m_w;
	}
	else {
		m_00 = v0.m_x;
		m_10 = v0.m_y;
		m_20 = v0.m_z;
		m_30 = v0.m_w;

		m_01 = v1.m_x;
		m_11 = v1.m_y;
		m_21 = v1.m_z;
		m_31 = v1.m_w;

		m_02 = v2.m_x;
		m_12 = v2.m_y;
		m_22 = v2.m_z;
		m_32 = v2.m_w;

		m_03 = v3.m_x;
		m_13 = v3.m_y;
		m_23 = v3.m_z;
		m_33 = v3.m_w;
	}	
}

CMatrix::CMatrix( float fDiag )
{
	SetIdentity();
	m_00 = fDiag;
	m_11 = fDiag;
	m_22 = fDiag;
	m_33 = fDiag;
}

bool CMatrix::IsIdentity() const
{
	return (m_00 == 1 && m_01 == 0 && m_02 == 0 && m_03 == 0 &&
			m_10 == 0 && m_11 == 1 && m_12 == 0 && m_13 == 0 &&
			m_20 == 0 && m_21 == 0 && m_22 == 1 && m_23 == 0 &&
			m_30 == 0 && m_31 == 0 && m_32 == 0 && m_33 == 1);
}

//-----------------------------------------------------------------------------------------------------
//											SetIdentity
//-----------------------------------------------------------------------------------------------------
void CMatrix::SetIdentity()
{
	m_00=1; m_01=0; m_02=0; m_03=0;
	m_10=0; m_11=1; m_12=0; m_13=0;
	m_20=0; m_21=0; m_22=1; m_23=0;
	m_30=0; m_31=0; m_32=0; m_33=1;
}


//-----------------------------------------------------------------------------------------------------
//											GetTranspose
//-----------------------------------------------------------------------------------------------------
CMatrix CMatrix::GetTranspose() const
{
	return CMatrix(
						m_00,m_10,m_20,m_30,
						m_01,m_11,m_21,m_31,
						m_02,m_12,m_22,m_32,
						m_03,m_13,m_23,m_33
						);
	
}


//-----------------------------------------------------------------------------------------------------
//											GetDeterminant
//-----------------------------------------------------------------------------------------------------
float CMatrix::GetDeterminant() const
{
	float A = m_11 * (m_22 * m_33 - m_32 * m_23) - m_21 * (m_12 * m_33 - m_32 * m_13) + m_31 * (m_12 * m_23 - m_22 * m_13);
	float B = m_01 * (m_22 * m_33 - m_32 * m_23) - m_21 * (m_02 * m_33 - m_32 * m_03) + m_31 * (m_02 * m_23 - m_22 * m_03);
	float C = m_01 * (m_12 * m_33 - m_32 * m_13) - m_11 * (m_02 * m_33 - m_32 * m_03) + m_31 * (m_02 * m_13 - m_12 * m_03);
	float D = m_01 * (m_12 * m_23 - m_22 * m_13) - m_11 * (m_02 * m_23 - m_22 * m_03) + m_21 * (m_02 * m_13 - m_12 * m_03);

	return m_00 * A - m_10 * B + m_20 * C - m_30 * D;
}


//-----------------------------------------------------------------------------------------------------
//											GetInverse
//-----------------------------------------------------------------------------------------------------
void CMatrix::GetInverse( CMatrix& oMat ) const
{
	if ( &oMat == this )
	{
		exception e( "CMatrix::GetInverse : Impossible de passer la matrice instance en argument (risque de modification de l'instance)" );
		throw e;
	}
	float Det = GetDeterminant();

    oMat.m_00 =  (m_11 * (m_22 * m_33 - m_23 * m_32) - m_21 * (m_12 * m_33 - m_32 * m_13) + m_31 * (m_12 * m_23 - m_22 *  m_13)) / Det;
    oMat.m_01 = -(m_01 * (m_22 * m_33 - m_32 * m_23) - m_21 * (m_02 * m_33 - m_32 * m_03) + m_31 * (m_02 * m_23 - m_22 *  m_03)) / Det;
    oMat.m_02 =  (m_01 * (m_12 * m_33 - m_32 * m_13) - m_11 * (m_02 * m_33 - m_32 * m_03) + m_31 * (m_02 * m_13 - m_12 *  m_03)) / Det;
    oMat.m_03 = -(m_01 * (m_12 * m_23 - m_22 * m_13) - m_11 * (m_02 * m_23 - m_22 * m_03) + m_21 * (m_02 * m_13 - m_12 *  m_03)) / Det;

    oMat.m_10 = -(m_10 * (m_22 * m_33 - m_23 * m_32) - m_12 * (m_20 * m_33 - m_23 * m_30) + m_13 * (m_20 * m_32 - m_22 *  m_30)) / Det;
    oMat.m_11 =  (m_00 * (m_22 * m_33 - m_23 * m_32) - m_02 * (m_20 * m_33 - m_23 * m_30) + m_03 * (m_20 * m_32 - m_22 *  m_30)) / Det;
    oMat.m_12 = -(m_00 * (m_12 * m_33 - m_13 * m_32) - m_02 * (m_10 * m_33 - m_13 * m_30) + m_03 * (m_10 * m_32 - m_12 *  m_30)) / Det;
    oMat.m_13 =  (m_00 * (m_12 * m_23 - m_13 * m_22) - m_02 * (m_10 * m_23 - m_13 * m_20) + m_03 * (m_10 * m_22 - m_12 *  m_20)) / Det;

    oMat.m_20 =  (m_10 * (m_21 * m_33 - m_23 * m_31) - m_11 * (m_20 * m_33 - m_23 * m_30) + m_13 * (m_20 * m_31 - m_21 *  m_30)) / Det;
    oMat.m_21 = -(m_00 * (m_21 * m_33 - m_23 * m_31) - m_01 * (m_20 * m_33 - m_23 * m_30) + m_03 * (m_20 * m_31 - m_21 *  m_30)) / Det;
    oMat.m_22 =  (m_00 * (m_11 * m_33 - m_13 * m_31) - m_01 * (m_10 * m_33 - m_13 * m_30) + m_03 * (m_10 * m_31 - m_11 *  m_30)) / Det;
    oMat.m_23 = -(m_00 * (m_11 * m_23 - m_13 * m_21) - m_01 * (m_10 * m_23 - m_13 * m_20) + m_03 * (m_10 * m_21 - m_11 *  m_20)) / Det;

    oMat.m_30 = -(m_10 * (m_21 * m_32 - m_22 * m_31) - m_11 * (m_20 * m_32 - m_22 * m_30) + m_12 * (m_20 * m_31 - m_21 *  m_30)) / Det;
    oMat.m_31 =  (m_00 * (m_21 * m_32 - m_22 * m_31) - m_01 * (m_20 * m_32 - m_22 * m_30) + m_02 * (m_20 * m_31 - m_21 *  m_30)) / Det;
    oMat.m_32 = -(m_00 * (m_11 * m_32 - m_12 * m_31) - m_01 * (m_10 * m_32 - m_12 * m_30) + m_02 * (m_10 * m_31 - m_11 *  m_30)) / Det;
    oMat.m_33 =  (m_00 * (m_11 * m_22 - m_12 * m_21) - m_01 * (m_10 * m_22 - m_12 * m_20) + m_02 * (m_10 * m_21 - m_11 *  m_20)) / Det;    
}


//-----------------------------------------------------------------------------------------------------
//											operator =
//-----------------------------------------------------------------------------------------------------
const CMatrix& CMatrix::operator =(const CMatrix& mat)
{
	m_00 = mat.m_00 ; m_01 = mat.m_01 ; m_02 = mat.m_02 ; m_03 = mat.m_03;
	m_10 = mat.m_10 ; m_11 = mat.m_11 ; m_12 = mat.m_12 ; m_13 = mat.m_13;
	m_20 = mat.m_20 ; m_21 = mat.m_21 ; m_22 = mat.m_22 ; m_23 = mat.m_23;
	m_30 = mat.m_30 ; m_31 = mat.m_31 ; m_32 = mat.m_32 ; m_33 = mat.m_33;
	return mat;
}


void CMatrix::operator =(const float * fValues)
{
	m_00 = fValues[0]; m_01 = fValues[1]; m_02 = fValues[2]; m_03 = fValues[3];
	m_10 = fValues[4]; m_11 = fValues[5]; m_12 = fValues[6]; m_13 = fValues[7];
	m_20 = fValues[8]; m_21 = fValues[9]; m_22 = fValues[10]; m_23 = fValues[11];
	m_30 = fValues[12]; m_31 = fValues[13]; m_32 = fValues[14]; m_33 = fValues[15];

}

//-----------------------------------------------------------------------------------------------------
//											operator +
//-----------------------------------------------------------------------------------------------------
CMatrix CMatrix::operator +(const CMatrix& mat) const
{
	return CMatrix(		m_00 + mat.m_00 , m_01 + mat.m_01 , m_02 + mat.m_02 , m_03 + mat.m_03 , 
						m_10 + mat.m_10 , m_11 + mat.m_11 , m_12 + mat.m_12 , m_13 + mat.m_13 , 
						m_20 + mat.m_20 , m_21 + mat.m_21 , m_22 + mat.m_22 , m_23 + mat.m_23 , 
						m_30 + mat.m_30 , m_31 + mat.m_31 , m_32 + mat.m_32 , m_33 + mat.m_33  	);
}

CMatrix CMatrix::operator -(const CMatrix& mat) const
{
	return CMatrix(		m_00 - mat.m_00 , m_01 - mat.m_01 , m_02 - mat.m_02 , m_03 - mat.m_03 , 
						m_10 - mat.m_10 , m_11 - mat.m_11 , m_12 - mat.m_12 , m_13 - mat.m_13 , 
						m_20 - mat.m_20 , m_21 - mat.m_21 , m_22 - mat.m_22 , m_23 - mat.m_23 , 
						m_30 - mat.m_30 , m_31 - mat.m_31 , m_32 - mat.m_32 , m_33 - mat.m_33  	);
}

//-----------------------------------------------------------------------------------------------------
//											operator *
//-----------------------------------------------------------------------------------------------------
CMatrix CMatrix::operator *(const CMatrix& mat) const
{
	CMatrix res;
	res.m_00 = m_00*mat.m_00 + m_01*mat.m_10 + m_02*mat.m_20 + m_03*mat.m_30;
	res.m_01 = m_00*mat.m_01 + m_01*mat.m_11 + m_02*mat.m_21 + m_03*mat.m_31;
	res.m_02 = m_00*mat.m_02 + m_01*mat.m_12 + m_02*mat.m_22 + m_03*mat.m_32;
	res.m_03 = m_00*mat.m_03 + m_01*mat.m_13 + m_02*mat.m_23 + m_03*mat.m_33;

	res.m_10 = m_10*mat.m_00 + m_11*mat.m_10 + m_12*mat.m_20 + m_13*mat.m_30;
	res.m_11 = m_10*mat.m_01 + m_11*mat.m_11 + m_12*mat.m_21 + m_13*mat.m_31;
	res.m_12 = m_10*mat.m_02 + m_11*mat.m_12 + m_12*mat.m_22 + m_13*mat.m_32;
	res.m_13 = m_10*mat.m_03 + m_11*mat.m_13 + m_12*mat.m_23 + m_13*mat.m_33;

	res.m_20 = m_20*mat.m_00 + m_21*mat.m_10 + m_22*mat.m_20 + m_23*mat.m_30;
	res.m_21 = m_20*mat.m_01 + m_21*mat.m_11 + m_22*mat.m_21 + m_23*mat.m_31;
	res.m_22 = m_20*mat.m_02 + m_21*mat.m_12 + m_22*mat.m_22 + m_23*mat.m_32;
	res.m_23 = m_20*mat.m_03 + m_21*mat.m_13 + m_22*mat.m_23 + m_23*mat.m_33;

	res.m_30 = m_30*mat.m_00 + m_31*mat.m_10 + m_32*mat.m_20 + m_33*mat.m_30;
	res.m_31 = m_30*mat.m_01 + m_31*mat.m_11 + m_32*mat.m_21 + m_33*mat.m_31;
	res.m_32 = m_30*mat.m_02 + m_31*mat.m_12 + m_32*mat.m_22 + m_33*mat.m_32;
	res.m_33 = m_30*mat.m_03 + m_31*mat.m_13 + m_32*mat.m_23 + m_33*mat.m_33;

	return res;
}



//-----------------------------------------------------------------------------------------------------
//											operator *
//	Renvoie le résultat de la multiplication de la matrice par le vecteur v
//-----------------------------------------------------------------------------------------------------
CVector CMatrix::operator *(const CVector& v) const
{
	return CVector(		m_00*v.m_x + m_01*v.m_y + m_02*v.m_z + m_03*v.m_w,
						m_10*v.m_x + m_11*v.m_y + m_12*v.m_z + m_13*v.m_w,
						m_20*v.m_x + m_21*v.m_y + m_22*v.m_z + m_23*v.m_w,
						m_30*v.m_x + m_31*v.m_y + m_32*v.m_z + m_33*v.m_w		);
}



//-----------------------------------------------------------------------------------------------------
//											operator *=
//-----------------------------------------------------------------------------------------------------
void CMatrix::operator *=(const CMatrix& mat)
{		
	(*this) = (*this) * mat;
}

void CMatrix::operator+=(const CMatrix& mat)
{
	(*this) = (*this) + mat;
}

CMatrix CMatrix::operator*( float f )
{
	return CMatrix( m_00 * f,	m_01 * f,	m_02 * f,	m_03 * f ,
					m_10 * f,	m_11 * f,	m_12 * f,	m_13 * f ,
					m_20 * f,	m_21 * f,	m_22 * f,	m_23 * f ,
					m_30 * f,	m_31 * f,	m_32 * f,	m_33 * f );
}

//-----------------------------------------------------------------------------------------------------
//											operator /
//-----------------------------------------------------------------------------------------------------
CMatrix CMatrix::operator /(float d)
{
	return CMatrix(		m_00/d		,	m_01/d		,	m_02/d		,	m_03/d		,	
						m_10/d		,	m_11/d		,	m_12/d		,	m_13/d		,
						m_20/d		,	m_21/d		,	m_22/d		,	m_23/d		,
						m_30/d		,	m_31/d		,	m_32/d		,	m_33/d		);

}

bool CMatrix::operator==(const CMatrix& mat) const
{
	return ( (m_00 == mat.m_00) && (m_01 == mat.m_01) && (m_02 == mat.m_02) && (m_03 == mat.m_03) &&
			 (m_10 == mat.m_10) && (m_11 == mat.m_11) && (m_12 == mat.m_12) && (m_13 == mat.m_13) &&
			 (m_20 == mat.m_20) && (m_21 == mat.m_21) && (m_22 == mat.m_22) && (m_23 == mat.m_23) &&
			 (m_30 == mat.m_30) && (m_31 == mat.m_31) && (m_32 == mat.m_32) && (m_33 == mat.m_33) );
}

//
////-----------------------------------------------------------------------------------------------------
////											glLoadMatrix
////-----------------------------------------------------------------------------------------------------
//void CMatrix::glLoadMatrix() const
//{
//	GLfloat params[16] = {	m_00	,	m_10	,	m_20	,	m_30 , 
//							m_01	,	m_11	,	m_21	,	m_31 , 
//							m_02	,	m_12	,	m_22	,	m_32 , 
//							m_03	,	m_13	,	m_23	,	m_33  	};
//	
//	glLoadMatrixf(params);
//}

//
////-----------------------------------------------------------------------------------------------------
////											glMultMatrix
////-----------------------------------------------------------------------------------------------------
//void CMatrix::glMultMatrix() const
//{
//	GLfloat params[16] = {	m_00	,	m_10	,	m_20	,	m_30 , 
//							m_01	,	m_11	,	m_21	,	m_31 , 
//							m_02	,	m_12	,	m_22	,	m_32 , 
//							m_03	,	m_13	,	m_23	,	m_33  	};
//	
//	glMultMatrixf(params);
//}

//
////-----------------------------------------------------------------------------------------------------
////											DrawFillRect
////-----------------------------------------------------------------------------------------------------
//void CMatrix::DrawFillRect() const
//{
//	glBegin(GL_LINE_LOOP);
//		glVertex3f(m_00 , m_10 , m_20);
//		glVertex3f(m_01 , m_11 , m_21);
//		glVertex3f(m_02 , m_12 , m_22);
//		glVertex3f(m_03 , m_13 , m_23);
//	glEnd();
//}


CMatrix CMatrix::GetRotation() const
{
	return CMatrix(m_00 	,	m_01	,	m_02	,	0 	,
				   m_10 	,	m_11	,	m_12	,	0 	,	
				   m_20 	,	m_21	,	m_22	,	0 	,	
					 0 		,	  0		,	  0		,	1 	);	
}


CMatrix CMatrix::GetTranslation()
{
	return CMatrix(		1.f	,	0.f	,	0.f	,	m_03	,	
						0.f	,	1.f	,	0.f	,	m_13	,	
						0.f	,	0.f	,	1.f	,	m_23	,	
						0.f	,	0.f	,	0.f	,	  1		);
}

CVector CMatrix::GetPosition() const
{
	return CVector( m_03,m_13,m_23,m_33 );
}

void CMatrix::GetPosition(float &x, float &y, float &z) const
{
	x = m_03;
	y = m_13; 
	z = m_23;
}

void CMatrix::GetPosition(CVector& pos) const
{
	pos.m_x = m_03;
	pos.m_y = m_13;
	pos.m_z = m_23;
}

void CMatrix::SetPosition( float x, float y, float z )
{
	m_03 = x;
	m_13 = y;
	m_23 = z;
	m_33 = 1.f;
}

void CMatrix::Get( float* arr ) const
{
	arr[ 0 ] 	= m_00; 	arr[ 1 ] 	= m_01; 	arr[ 2 ] 	= m_02; 	arr[ 3 ] 	= m_03;
	arr[ 4 ] 	= m_10; 	arr[ 5 ] 	= m_11; 	arr[ 6 ] 	= m_12; 	arr[ 7 ] 	= m_13;
	arr[ 8 ] 	= m_20; 	arr[ 9 ] 	= m_21; 	arr[ 10 ] 	= m_22; 	arr[ 11 ] 	= m_23;
	arr[ 12 ]	= m_30; 	arr[ 13 ]	= m_31; 	arr[ 14 ] 	= m_32; 	arr[ 15 ] 	= m_33;
}

void CMatrix::Get( vector< float >& vData ) const
{
	vData.push_back( m_00 ); vData.push_back( m_01 ); vData.push_back( m_02 ); vData.push_back( m_03 );
	vData.push_back( m_10 ); vData.push_back( m_11 ); vData.push_back( m_12 ); vData.push_back( m_13 );
	vData.push_back( m_20 ); vData.push_back( m_21 ); vData.push_back( m_22 ); vData.push_back( m_23 );
	vData.push_back( m_30 ); vData.push_back( m_31 ); vData.push_back( m_32 ); vData.push_back( m_33 );
}

void CMatrix::Set( const float* arr )
{
	m_00 = arr[ 0 ]; 	m_01 = arr[ 1 ]; 	m_02 = arr[ 2 ]; 	m_03 = arr[ 3 ];
	m_10 = arr[ 4 ]; 	m_11 = arr[ 5 ]; 	m_12 = arr[ 6 ]; 	m_13 = arr[ 7 ];
	m_20 = arr[ 8 ]; 	m_21 = arr[ 9 ]; 	m_22 = arr[ 10 ]; 	m_23 = arr[ 11 ];
	m_30 = arr[ 12 ]; 	m_31 = arr[ 13 ]; 	m_32 = arr[ 14 ]; 	m_33 = arr[ 15 ];
}

void CMatrix::Set( const vector< float >& vData )
{
	Set( &vData[ 0 ] );
}

//-----------------------------------------------------------------------------------------------------
//											GetInf
//-----------------------------------------------------------------------------------------------------
void CMatrix::GetInf(float& nInfx,float& nInfy,float& nInfz) const
{		
	nInfx = m_00;
	nInfy = m_10;
	nInfz = m_20;
	
	if (nInfx > m_01) nInfx = m_01;
	if (nInfx > m_02) nInfx = m_02;
	if (nInfx > m_03) nInfx = m_03;		
	
	if (nInfy > m_11) nInfy = m_11;
	if (nInfy > m_12) nInfy = m_12;
	if (nInfy > m_13) nInfy = m_13;
	
	if (nInfz > m_21) nInfz = m_21;
	if (nInfz > m_22) nInfz = m_22;
	if (nInfz > m_23) nInfz = m_23;
}


//-----------------------------------------------------------------------------------------------------
//											GetSup
//-----------------------------------------------------------------------------------------------------
void CMatrix::GetSup(float& nSupx,float& nSupy,float& nSupz) const
{
	nSupx = m_00;
	nSupy = m_10;
	nSupz = m_20;
	
	if (nSupx < m_01) nSupx = m_01;
	if (nSupx < m_02) nSupx = m_02;
	if (nSupx < m_03) nSupx = m_03;		
	
	if (nSupy < m_11) nSupy = m_11;
	if (nSupy < m_12) nSupy = m_12;
	if (nSupy < m_13) nSupy = m_13;
	
	if (nSupz < m_21) nSupz = m_21;
	if (nSupz < m_22) nSupz = m_22;
	if (nSupz < m_23) nSupz = m_23;
}


CMatrix CMatrix::GetxRotation()
{
	return CMatrix(		1	,	0		,	0		,	0	,	
						0	,	m_11	,	m_12	,	0	,
						0	,	m_21	,	m_22	,	0	,	
						0	,	0		,	0		,	1);
					
}

CMatrix CMatrix::GetyRotation()
{
	return CMatrix(		m_00	,	0		,	m_02	,	0	,	
						0		,	1		,	0		,	0	,
						m_20	,	0		,	m_22	,	0	,	
						0		,	0		,	0		,	1);
					
}

#if 0
void CMatrix::GetQuaternion( CQuaternion& quat ) const
{
	float fTrace = m_00 + m_11 + m_22 + 1;
	if( fTrace > 3.9999 )
		quat.Fill( 0, 0, 0, 1 );
	else
	{
		if ( fTrace > 0.0001 )
		{
			float S = sqrt( fTrace ) * 2;
			quat.Fill( ( m_21 - m_12 ) / S, ( m_02 - m_20 ) / S, ( m_10 - m_01 ) / S, 0.25f * S );
		}
		else 
		{
			if( ( abs( m_00 ) > abs( m_11 ) ) && ( abs( m_00 ) > abs( m_22 ) ) )
			{
				if ( ( 1.0 + m_00 - m_11 - m_22 ) <= 0 )
					throw 1;
				float S = sqrt( 1.0 + m_00 - m_11 - m_22 ) * 2.f; // S=4*qx 
				quat.Fill( 0.25 * S, ( m_01 + m_10 ) / S, (m_02 + m_20 ) / S, ( m_21 - m_12 ) / S );
			} 
			else if ( abs( m_11 ) > abs( m_22 ) ) 
			{ 
				if ( ( 1.f + m_11 - m_00 - m_22 ) <= 0.f )
					throw 1;
				float S = sqrt( 1.f + m_11 - m_00 - m_22 ) * 2; // S=4*qy
				quat.Fill( ( m_01 + m_10 ) / S, 0.25f * S, ( m_12 + m_21 ) / S, ( m_02 - m_20 ) / S );
			}
			else
			{
				if ( ( 1.f + m_22 - m_00 - m_11 ) <= 0.f )
				{
					MessageBox( NULL, "CMatrix::GetQuaternion() : 1.f + m_22 - m_00 - m_11 ) est négatif ou nul ", "", MB_ICONERROR );
				}
				else
				{
					float S = sqrt( 1.f + m_22 - m_00 - m_11 ) * 2.f; // S=4*qz
					quat.Fill( ( m_02 + m_20 ) / S, ( m_12 + m_21 ) / S, 0.25f * S, ( m_10 - m_01 ) / S );
				}
			}
		}
	}
	quat.m_vPosition = CVector( m_03, m_13, m_23, m_33 );
	quat.Normalize();
}
#endif // 0

void CMatrix::GetQuaternion( CQuaternion& quat ) const
{
	float fTrace = m_00 + m_11 + m_22 + 1;
	float S = 0;
	if ( fTrace > 0.f )
	{
		S = 0.5f / sqrt( fTrace );
		quat.Fill( ( m_21 - m_12 ) * S, ( m_02 - m_20 ) * S, ( m_10 - m_01 ) * S, 0.25f / S );
	}
	else 
	{
		if( ( m_00 > m_11 ) && ( m_00 > m_22 ) )
		{
			S = sqrt( 1 + m_00 - m_11 - m_22 ) * 2.f;
			quat.Fill( 0.25f * S, ( m_01 + m_10 ) / S, ( m_02 + m_20 ) / S, ( m_12 - m_21 ) / S );
		}
		else if( ( m_11 > m_00 ) && ( m_11 > m_22 ) )
		{
			S =  sqrt( 1 - m_00 + m_11 - m_22 ) * 2.f;
			quat.Fill( ( m_01 + m_10 ) / S, 0.25f * S, ( m_12 + m_21 ) / S, ( m_02 - m_20 ) / S );
		}
		else if( ( m_22 > m_00 ) && ( m_22 > m_11 ) )
		{
			S = sqrt( 1 - m_00 - m_11 + m_22 ) * 2.f;
			quat.Fill( ( m_02 + m_20 ) / S, ( m_12 + m_21 ) / S, 0.25f * S, ( m_01 - m_10 ) / S );
		}
	}
}

//-----------------------------------------------------------------------------------------------------
//											sGetTranslation
//-----------------------------------------------------------------------------------------------------
CMatrix CMatrix::GetTranslation(float x,float y,float z)
{
	return CMatrix(		1	,	0	,	0	,	x	,	
						0	,	1	,	0	,	y	,
						0	,	0	,	1	,	z	,	
						0	,	0	,	0	,	1		);
}


//-----------------------------------------------------------------------------------------------------
//											sGetTranslation
//-----------------------------------------------------------------------------------------------------
CMatrix CMatrix::GetTranslation(const CVector& vTranslation)
{
	return CMatrix(		1	,	0	,	0	,	vTranslation.m_x	,	
						0	,	1	,	0	,	vTranslation.m_y	,
						0	,	0	,	1	,	vTranslation.m_z	,	
						0	,	0	,	0	,			1				);
}


//-----------------------------------------------------------------------------------------------------
//											sGetxRotation
//-----------------------------------------------------------------------------------------------------
CMatrix CMatrix::GetxRotation(float Angle)
{
	float fAngleRad = Angle * 3.1415927f / 180.f;
	return CMatrix(
							1		,		0		,			0			,		0,

							0		,	cosf (fAngleRad),		-sinf(fAngleRad)	,		0,

							0		,	sinf(fAngleRad)	,		cosf(fAngleRad)		,		0,

							0		,		0		,			0			,		1

					);
}


//-----------------------------------------------------------------------------------------------------
//											sGetyRotation
//-----------------------------------------------------------------------------------------------------
CMatrix CMatrix::GetyRotation(float Angle)
{
	float fAngleRad = Angle * 3.1415927f / 180.f;
	return CMatrix(
						cosf(fAngleRad)		,		0		,		sinf(fAngleRad)	,		0,

							0			,		1		,			0		,		0,

						-sinf(fAngleRad)	,		0		,		cosf(fAngleRad)	,		0,

							0			,		0		,			0		,		1
					);
}

//-----------------------------------------------------------------------------------------------------
//											sGetzRotation
//-----------------------------------------------------------------------------------------------------
CMatrix CMatrix::GetzRotation(float Angle)
{
	float fAngleRad = Angle * (float)M_PI / 180.f;
	return CMatrix(
						cosf(fAngleRad)		,	-sinf(fAngleRad),		0		,		0,

						sinf(fAngleRad)		,	cosf(fAngleRad)	,		0		,		0,

							0				,		0			,		1		,		0,

							0				,		0			,		0		,		1
						
					);
}

void CMatrix::Slerp( const CMatrix& oInitial, const CMatrix& oFinal, float t, CMatrix& oResult )
{
	CQuaternion qInitial, qFinal, qResult;
	oInitial.GetQuaternion( qInitial );
	oFinal.GetQuaternion( qFinal );
	CQuaternion::Slerp( qInitial, qFinal, t, qResult );
	qResult.Normalize();
	qResult.GetMatrix( oResult );
	LerpAffinePart( oInitial, oFinal, t, oResult );
}

void CMatrix::LerpAffinePart( const CMatrix& oInitial, const CMatrix& oFinal, float t, CMatrix& oResult )
{
	oResult.m_03 = oInitial.m_03 + ( oFinal.m_03 - oInitial.m_03 ) * t;
	oResult.m_13 = oInitial.m_13 + ( oFinal.m_13 - oInitial.m_13 ) * t;
	oResult.m_23 = oInitial.m_23 + ( oFinal.m_23 - oInitial.m_23 ) * t;
}

void CMatrix::GetPassage( const CMatrix& mInitial, const CMatrix& mFinal, CMatrix& mPassage )
{
	CMatrix mInitialInverse;
	mInitial.GetInverse( mInitialInverse );
	mPassage = mFinal * mInitialInverse;
}

//void CMatrix::SlerpInvariantAxis( const CMatrix& oInitial, const CMatrix& oFinal, float t, CMatrix& oResult )
//{
//	CQuaternion qInitial, qFinal, qResult;
//	oInitial.GetQuaternion( qInitial );
//	oFinal.GetQuaternion( qFinal );
//	CQuaternion::SlerpInvariantAxis( qInitial, qFinal, t, qResult );
//	qResult.GetMatrix( oResult );
//	LerpAffinePart( oInitial, oFinal, t, oResult );
//	//oResult.SetAffinePart( oInitial );
//}

void CMatrix::SetAffinePart( const CMatrix& mat )
{
	m_03 = mat.m_03;
	m_13 = mat.m_13;
	m_23 = mat.m_23;
}

void CMatrix::SetAffinePart( float x, float y, float z )
{
	m_03 = x;
	m_13 = y;
	m_23 = z;
}

void CMatrix::SetAffinePart( const CVector& v )
{
	m_03 = v.m_x;
	m_13 = v.m_y;
	m_23 = v.m_z;
}

void CMatrix::AddTranslation( const CVector& v )
{
	m_03 += v.m_x;
	m_13 += v.m_y;
	m_23 += v.m_z;
}

void CMatrix::AddTranslation( float x, float y, float z )
{
	m_03 += x;
	m_13 += y;
	m_23 += z;
}

void CMatrix::GetAffinePart( CVector& v )
{
	v.m_x = m_03;
	v.m_y = m_13;
	v.m_z = m_23;
}

void CMatrix::GetxBase(CVector& v) const
{
	v.m_x = m_00;
	v.m_y = m_01;
	v.m_z = m_02;
	v.m_w = 1.f;
}

void CMatrix::GetyBase(CVector& v) const
{
	v.m_x = m_10;
	v.m_y = m_11;
	v.m_z = m_12;
	v.m_w = 1.f;
}

void CMatrix::GetzBase(CVector& v) const
{
	v.m_x = m_20;
	v.m_y = m_21;
	v.m_z = m_22;
	v.m_w = 1.f;
}

void CMatrix::GetAxeAngleRotation( float x, float y, float z, float fAngle, CMatrix& mOut )
{
	float c = cos( fAngle );
	float s = sin( fAngle );
	mOut.Fill(	x+(1-x*x)*c,		x*y*(1-c)-z*s,		x*z*(1-c)+y*s,			0,
				x*y*(1-c)+z*s,		y*y+(1-y*y)*c,		y*z*(1-c)-x*s,			0,
				x*z*(1-c)-y*s,		y*z*(1-c)+x*s,		z*z+(1-z*z)*c,			0,
					0,					0,					0,					1 );
}

void CMatrix::Fill(	float a00, float a01, float a02, float a03, 
					float a10, float a11, float a12, float a13,
					float a20, float a21, float a22, float a23, 
					float a30, float a31, float a32, float a33 )
{
	m_00 = a00;
	m_01 = a01;
	m_02 = a02;
	m_03 = a03;

	m_10 = a10;
	m_11 = a11;
	m_12 = a12;
	m_13 = a13;

	m_20 = a20;
	m_21 = a21;
	m_22 = a22;
	m_23 = a23;

	m_30 = a30;
	m_31 = a31;
	m_32 = a32;
	m_33 = a33;
}

void CMatrix::GetInterpolationMatrix( const CMatrix& oLast, const CMatrix& oNext, CMatrix& oResult, float t )
{
	CMatrix oLastInv;
	oLast.GetInverse( oLastInv );
	CMatrix P = oLastInv * oNext;
	CMatrix Pt;
	CQuaternion qr;
	P.GetQuaternion(qr);
	CVector vAxis;
	float fAngle;
	qr.GetAngleAxis(vAxis, fAngle );
	CQuaternion qi( vAxis, fAngle * t );			
	qi.GetMatrix(Pt);
	oResult = oLast * Pt;
}

const IPersistantObject& CMatrix::operator >> (CBinaryFileStorage& store) const
{
	vector< float > vData;
	this->Get(vData);
	store << vData;
	return *this;
}

IPersistantObject& CMatrix::operator << (const CBinaryFileStorage& store)
{
	vector< float > vData;
	store >> vData;
	this->Set(vData);
	return *this;
}

const IPersistantObject& CMatrix::operator >> (CAsciiFileStorage& store) const
{
	CStringStorage oString;
	oString.SetCap(store.GetCap());
	oString.SetPrecision(store.GetPrecision());
	oString.SetWidth(15);
	oString << m_00 << m_01 << m_02 << m_03 << "\n";
	oString << m_10 << m_11 << m_12 << m_13 << "\n";
	oString << m_20 << m_21 << m_22 << m_23 << "\n";
	oString << m_30 << m_31 << m_32 << m_33 << "\n";
	store << oString.GetValue();
	return *this;
}

IPersistantObject& CMatrix::operator << (const CAsciiFileStorage& store)
{
	return *this;
}

const IPersistantObject& CMatrix::operator >> (CStringStorage& store) const
{
	return *this;
}

IPersistantObject& CMatrix::operator << (const CStringStorage& store)
{
	return *this;
}

CMatrix2X2::CMatrix2X2()
{
	SetIdentity();
}

CMatrix2X2::CMatrix2X2( float a00, float a01, float a02, 
						float a10, float a11, float a12,
						float a20, float a21, float a22	)
{
	m_00 = a00;
	m_01 = a01;
	m_02 = a02;
	m_10 = a10;
	m_11 = a11;
	m_12 = a12;
	m_20 = a20;
	m_21 = a21;
	m_22 = a22;
}

void CMatrix2X2::SetIdentity()
{
	m_00 = 1.f;
	m_01 = 0.f;
	m_02 = 0.f;
	m_10 = 0.f; 
	m_11 = 1.f;
	m_12 = 0.f;
	m_20 = 0.f;
	m_21 = 0.f;
	m_22 = 1.f;
}

CMatrix2X2 CMatrix2X2::GetRotation( float Angle )
{
	float fAngleRad = Angle * (float)M_PI / 180.f;
	return CMatrix2X2(	cosf(fAngleRad)		,	-sinf(fAngleRad), 0, 
						sinf(fAngleRad)		,	cosf(fAngleRad) , 0,
								0			,			0		, 1 ); 
}

CVector2D CMatrix2X2::operator*( const CVector2D& v ) const
{
	return CVector2D(	m_00 * v.m_x + m_01 * v.m_y + m_02 * v.m_w, 
						m_10 * v.m_x + m_11 * v.m_y + m_12 * v.m_w );
}

void CMatrix2X2::GetInverse( CMatrix2X2& oMatrixInv ) const
{
	float det = GetDeterminant();
	oMatrixInv.m_00 = ( m_11 * m_22 - m_12 * m_21 ) / det;
	oMatrixInv.m_10 = -( m_10 * m_22 - m_12 * m_20 ) / det;
	oMatrixInv.m_20 = ( m_10 * m_21 - m_11 * m_20 ) / det;
	oMatrixInv.m_01 = - ( m_01 * m_22 - m_02 * m_21 ) / det;
	oMatrixInv.m_11 = ( m_00 * m_22 - m_02 * m_20 ) / det;
	oMatrixInv.m_21 = -( m_00 * m_21 - m_01 * m_20 ) / det;
	oMatrixInv.m_02 = ( m_01 * m_12 - m_02 * m_11 ) / det;
	oMatrixInv.m_12 = -( m_00 * m_12 - m_02 * m_10 ) / det;
	oMatrixInv.m_22 = ( m_00 * m_11 - m_01 * m_10 ) / det;
}

void CMatrix2X2::AddTranslation( const CVector2D& v )
{
	m_02 += v.m_x;
	m_12 += v.m_y;
}

float CMatrix2X2::GetDeterminant()const
{
	return m_00 * ( m_11 * m_22 - m_12 * m_21 ) - m_01 * ( m_10 * m_22 - m_12 * m_20 ) + m_02 * ( m_10 * m_21 - m_11 * m_20 );
}

CMatrix2X2 CMatrix2X2::operator*(const CMatrix2X2& mat) const
{
	CMatrix2X2 res;
	res.m_00 = m_00*mat.m_00 + m_01*mat.m_10 + m_02*mat.m_20;
	res.m_01 = m_00*mat.m_01 + m_01*mat.m_11 + m_02*mat.m_21;
	res.m_02 = m_00*mat.m_02 + m_01*mat.m_12 + m_02*mat.m_22;

	res.m_10 = m_10*mat.m_00 + m_11*mat.m_10 + m_12*mat.m_20;
	res.m_11 = m_10*mat.m_01 + m_11*mat.m_11 + m_12*mat.m_21;
	res.m_12 = m_10*mat.m_02 + m_11*mat.m_12 + m_12*mat.m_22;

	res.m_20 = m_20*mat.m_00 + m_21*mat.m_10 + m_22*mat.m_20;
	res.m_21 = m_20*mat.m_01 + m_21*mat.m_11 + m_22*mat.m_21;
	res.m_22 = m_20*mat.m_02 + m_21*mat.m_12 + m_22*mat.m_22;

	return res;
}