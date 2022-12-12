#version 400 core

uniform sampler2D 	mainTex;
uniform int useTexture;
uniform vec2 screenSize;

in Vertex
{
	vec4 colour;
	vec2 texCoord;
	vec3 position;
} IN;

out vec4 fragColor;

void main(void)
{
	if(useTexture == 0) {
		fragColor = IN.colour;
	}
	else {
		float alpha = texture(mainTex, IN.texCoord).r;
		
		if(alpha < 0.00001f) {
			discard;
		}
		
		fragColor = IN.colour * vec4(1,1,1,alpha);
	}
	
	//float lowerBoundX = (screenSize.x / 2) - 3.0f;
	//float upperBoundX = (screenSize.x / 2) + 3.0f;
	//float lowerBoundY = (screenSize.y / 2) - 3.0f;
	//float upperBoundY = (screenSize.y / 2) + 3.0f;
	
	//if(IN.position.x < lowerBoundX){
	//	fragColor = vec4(0,0,1,0);
	//}
	//if(IN.position.x > upperBoundX){
	//	fragColor = vec4(0,1,0,0);
	//}
	
	//if(		(IN.position.x >= lowerBoundX && IN.position.x <= upperBoundX) 
	//	&& 	(IN.position.y >= lowerBoundY && IN.position.y <= upperBoundY)){
	//	fragColor = vec4(0,0,0,0);
	//}
	
	//fragColor = vec4(IN.position.x, IN.position.y, IN.position.z, 1);
}