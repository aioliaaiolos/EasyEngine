varying vec4 vModelVertexPos;
varying vec3 N;
varying vec3 V;
varying vec2 Texcoord;


varying vec4 VertexColor;

uniform sampler2D heightMap;
uniform float groundHeight;


void main()
{
	Texcoord    = gl_MultiTexCoord0.xy;
	vec4 tVertex = gl_Vertex;
	
	vec4 texel;
	texel = texture2D( heightMap, gl_MultiTexCoord0.xy );
	tVertex.y = groundHeight * (texture2D( heightMap, gl_MultiTexCoord0.xy )-0.5);
	
	// test	
	/*
	if( (gl_MultiTexCoord0.x > 1.) || (gl_MultiTexCoord0.x < 0.)){
		tVertex.y = -999999999;
	}
	
	if(gl_MultiTexCoord0.y > 1. || gl_MultiTexCoord0.y < 0.){
		tVertex.y = -999999999;
	}*/
	// fin test
	
	
	vModelVertexPos = gl_ModelViewMatrix * tVertex;
	vec4 vViewVertexPos = gl_ProjectionMatrix * vModelVertexPos;
	N = normalize(gl_NormalMatrix * gl_Normal);
	V = -normalize(vModelVertexPos.xyz);
	
	

	gl_Position = vViewVertexPos;
	
	
	
	//VertexColor = texture2D( heightMap, Texcoord );
	//gl_Position.y = 0.;
}