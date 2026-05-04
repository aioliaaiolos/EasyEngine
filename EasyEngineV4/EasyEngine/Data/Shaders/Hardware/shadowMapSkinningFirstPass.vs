attribute vec4 vWeightedVertexID;
attribute vec4 vVertexWeight;
uniform mat4 matBones[85];

void main()
{
	mat4 matWeight = mat4(0.);
	for ( int iBone = 0; iBone < 4; iBone++ )
	{
		float fBoneID = vWeightedVertexID[ iBone ];
		if ( fBoneID != -1. )
		{
			float fWeightedVertexValue = vVertexWeight[ iBone ];
			matWeight += fWeightedVertexValue * matBones[ int(fBoneID) ];
		}
	}
    vec4 transformedVertex = matWeight * gl_Vertex;
	gl_Position = gl_ProjectionMatrix * gl_ModelViewMatrix * transformedVertex;
}
