#define MULTIMATERIAL

varying vec4 vVertexPos;
varying vec3 N;
varying vec3 V;
varying vec2 Texcoord;

#ifdef MULTIMATERIAL
varying float nPSMatID;
#endif // MULTIMATERIAL

attribute vec4 vVertexWeight;
attribute vec4 vWeightedVertexID;
#ifdef MULTIMATERIAL
attribute int nMatID;
#endif // MULTIMATERIAL

uniform mat4 matBones[80];

void main()
{
#ifdef MULTIMATERIAL
	nPSMatID = nMatID;
#endif // MULTIMATERIAL

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
	
	vVertexPos = gl_ModelViewMatrix * gl_Vertex;
	N = normalize(gl_NormalMatrix * gl_Normal);
	N = mat3(matWeight) * N;
	V = -normalize(vVertexPos.xyz);
	
	Texcoord    = gl_MultiTexCoord0.xy;
	
	gl_Position = gl_ProjectionMatrix * gl_ModelViewMatrix * matWeight * gl_Vertex;



}