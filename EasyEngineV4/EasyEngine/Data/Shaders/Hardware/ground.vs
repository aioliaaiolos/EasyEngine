varying vec4 vModelVertexPos;
varying vec3 N;
varying vec3 V;
varying vec2 Texcoord;
varying vec4 fragPosLightSpace;

uniform sampler2D heightMap;
uniform float groundHeight;
uniform mat4 lightSpaceModelview;
uniform mat4 lightSpaceProjection;


void main()
{
	Texcoord    = gl_MultiTexCoord0.xy;
	vec4 tVertex = gl_Vertex;
	tVertex.y = groundHeight * (texture2D( heightMap, gl_MultiTexCoord0.xy ).r - 0.5);
	
	fragPosLightSpace = lightSpaceProjection * lightSpaceModelview * tVertex;
	
	vModelVertexPos = gl_ModelViewMatrix * tVertex;
	gl_Position = gl_ProjectionMatrix * vModelVertexPos;
	N = normalize(gl_NormalMatrix * gl_Normal);
	V = -normalize(vModelVertexPos.xyz);
}
