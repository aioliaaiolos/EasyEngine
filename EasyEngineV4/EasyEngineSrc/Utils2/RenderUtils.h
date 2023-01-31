#include "math/Vector.h"

// stl
#include <string>

using namespace std;

class IRenderer;
class ILoaderManager;
class IEntity;
class IGUIManager;
class EEInterface;

class CRenderUtils
{
public:
	static void DrawBox( const CVector& oMinPoint, const CVector& oDimension, IRenderer& oRenderer  );
	static void ScreenCapture( string sFileName, EEInterface* pInterface, int x = 0, int y = 0, unsigned int h = -1, unsigned int w = -1);
	static void	IndexGeometry(	vector< unsigned int >& vIndexArray, vector< float >& vVertexArray, vector< unsigned int >& vUVIndexArray, 
								vector< float >& vUVVertexArray, vector< float >& vNormalVertexArray, vector< float >& vIndexedNormalVertexArray, 
								vector< float >& m_vWeightVertex, vector< float >& m_vWeigtedVertexID, bool& bHasIsolatedVertex );
	static void	CreateNonIndexedVertexArray( const std::vector< unsigned int >& vIndexArray, const std::vector< float >& vVertexArray, int nComposantCount, std::vector< float >& vOutVertexArray );
	static void CreateIndexedNormalArray( vector< float >& vVertexArray, vector< unsigned int >& vIndexArray, const vector< float >& vNonIndexedVertexArray, vector< float >& vIndexedVertexArray, 
										vector< float >& vWeightVertex, vector< float >& vWeigtedVertexID, bool& bHasIsolatedVertex );
};