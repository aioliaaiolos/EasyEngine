#define MULTIMATERIAL

varying vec4 vModelVertexPos;
varying vec3 N;
varying vec3 V;
varying vec2 Texcoord;

#ifdef MULTIMATERIAL
attribute int nMatID;
varying float fPSMatID;
#endif // MULTIMATERIAL


void main()
{
#ifdef MULTIMATERIAL
	fPSMatID = nMatID;
#endif // MULTIMATERIAL
	vModelVertexPos = gl_ModelViewMatrix * gl_Vertex;
	vec4 vViewVertexPos = gl_ProjectionMatrix * vModelVertexPos;
	N = normalize(gl_NormalMatrix * gl_Normal);
	V = -normalize(vModelVertexPos.xyz);
	Texcoord    = gl_MultiTexCoord0.xy;	
	gl_Position = vViewVertexPos;
}