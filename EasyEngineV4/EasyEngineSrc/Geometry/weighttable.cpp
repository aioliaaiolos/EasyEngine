#include "WeightTable.h"
#include "Exception.h"


void CWeightTable::Add( int iVertexIndex, int nBoneID, float fBoneWeight )
{
	m_mVertex[ iVertexIndex ][ nBoneID ] = fBoneWeight;
}

bool CWeightTable::Get( int iVertexIndex, std::map< int, float >& mWeight ) const
{
	map< int, map< int, float > >::const_iterator itMapWeight = m_mVertex.find( iVertexIndex );
	if( itMapWeight != m_mVertex.end() )
	{
		mWeight = itMapWeight->second;
		return true;
	}
	else
		return false;
}

int	 CWeightTable::GetVertexCount() const
{ 
	return (int)m_mVertex.size(); 
}

void CWeightTable::GetArrays( vector<float >& vWeightVertex, vector< float >& vWeigtedVertexID )
{
	int nWeightVertexCount = GetVertexCount();
	for ( int iVertex = 0; iVertex < nWeightVertexCount; iVertex++ )
	{
		map< int, float > mBone;
		if( Get( iVertex, mBone ) )
		{
			for ( map< int, float >::const_iterator itBone = mBone.begin(); itBone != mBone.end(); ++itBone )
			{
				vWeigtedVertexID.push_back( itBone->first );
				vWeightVertex.push_back( itBone->second );
			}
			for ( int iSupBone = (int)mBone.size(); iSupBone < 4; iSupBone++ )
			{
				vWeigtedVertexID.push_back( -1.f );
				vWeightVertex.push_back( 0.f );
			}
		}
		else
		{
			std::ostringstream oss;
			oss << "Erreur : le vertex " << iVertex << " n'est pas skinné";
			CEException e(oss.str());
			throw e;
		}
	}
}

void CWeightTable::BuildFromArrays( const vector<float >& vWeightVertex, const vector< float >& vWeightedVertexID )
{
	int nVertexCount = vWeightVertex.size() / 4;
	for( int iVertex = 0; iVertex <  nVertexCount; iVertex++ )
	{
		for( int j = 0; j < 4; j++ )
		{
			int nBoneID = vWeightedVertexID[ iVertex * 4 + j ];
			if( nBoneID != -1 )
			{
				float fWeight = vWeightVertex[ iVertex * 4 + j ];
				Add( iVertex, nBoneID, fWeight );
			}
		}
	}
}