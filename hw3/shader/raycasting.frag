#version 400

in vec3 EntryPoint;
in vec4 ExitPointCoord;

uniform sampler2D ExitPoints;
uniform sampler3D VolumeTex;
uniform sampler1D TransferFunc;  
uniform float     StepSize;
uniform vec2      ScreenSize;
layout (location = 0) out vec4 FragColor;

void main()
{
    vec3 exitPoint = texture(ExitPoints, gl_FragCoord.st/ScreenSize).xyz;
    if (EntryPoint == exitPoint)
    	discard;
    vec3 dir = exitPoint - EntryPoint;
    float len = length(dir);
    vec3 deltaDir = normalize(dir) * StepSize;
    float deltaDirLen = length(deltaDir);
    vec3 voxelCoord = EntryPoint;
    vec4 colorAcum = vec4(0.0);
    float alphaAcum = 0.0;
	
    float intensity;
    float lengthAcum = 0.0;
    vec4 colorSample; // The src color 
    float alphaSample; // The src alpha
    // backgroundColor
    vec4 bgColor = vec4( 1.0, 1.0, 1.0, 0.0);
 
    for(int i = 0; i < 1600; i++)
    {
    	intensity =  texture(VolumeTex, voxelCoord).x;
    	colorSample = texture(TransferFunc, intensity);
		
    	if (colorSample.a > 0.0) {
    	    colorSample.a = 1.0 - pow(1.0 - colorSample.a, StepSize*200.0f);
    	    colorAcum.rgb += (1.0 - colorAcum.a) * colorSample.rgb * colorSample.a;
    	    
			if (voxelCoord.x<0.5&&voxelCoord.y<0.5&&voxelCoord.z<0.5) {
				colorAcum.a = 0.0;
			}
			else{
				colorAcum.a += (1.0 - colorAcum.a) * colorSample.a;
			}
			
    	}
    	voxelCoord += deltaDir;
    	lengthAcum += deltaDirLen;
    	if (lengthAcum >= len )
    	{	
    	    colorAcum.rgb = colorAcum.rgb*colorAcum.a + (1 - colorAcum.a)*bgColor.rgb;		
    	    break;	
    	}	
    	else if (colorAcum.a > 1.0)
    	{
    	    colorAcum.a = 1.0;
    	    break;
    	}
    }
    FragColor = colorAcum;
	

	
    // for test
	// FragColor = vec4( 0.5, 1.0 ,0.3, 1.0);
    // FragColor = vec4(EntryPoint, 1.0);
    // FragColor = vec4(exitPoint, 1.0);
   
}
