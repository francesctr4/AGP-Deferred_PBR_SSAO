///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

#ifdef SSAO

#if defined(VERTEX) ///////////////////////////////////////////////////

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;

out vec2 TexCoords;

void main()
{
    TexCoords = aTexCoords;
    gl_Position = vec4(aPos, 1.0);
}

#elif defined(FRAGMENT) ///////////////////////////////////////////////

out float FragColor;

in vec2 TexCoords;

uniform sampler2D gPosition;    // World-space position from G-buffer
uniform sampler2D gNormal;      // World-space normal from G-buffer
uniform sampler2D texNoise;

uniform vec3 samples[64];
uniform int kernelSize;
uniform float radius;
uniform float bias;
uniform float SCREEN_WIDTH;
uniform float SCREEN_HEIGHT;

// Key addition: view/projection matrices
uniform mat4 view;             // Camera's view matrix
uniform mat4 projection;       // Camera's projection matrix

void main()
{
    const vec2 noiseScale = vec2(SCREEN_WIDTH/4.0, SCREEN_HEIGHT/4.0); 

    // 1. Convert world-space position/normal to view space
    vec3 worldPos = texture(gPosition, TexCoords).xyz;
    vec3 viewPos = vec3(view * vec4(worldPos, 1.0));  // World to view space
    
    vec3 worldNormal = normalize(texture(gNormal, TexCoords).rgb);
    mat3 normalMatrix = transpose(inverse(mat3(view)));  // Convert normal to view space
    vec3 viewNormal = normalize(normalMatrix * worldNormal);

    // 2. Create TBN matrix in view space
    vec3 randomVec = normalize(texture(texNoise, TexCoords * noiseScale).xyz);
    vec3 tangent = normalize(randomVec - viewNormal * dot(randomVec, viewNormal));
    vec3 bitangent = cross(viewNormal, tangent);
    mat3 TBN = mat3(tangent, bitangent, viewNormal);

    // 3. Calculate occlusion in view space
    float occlusion = 0.0;
    for(int i = 0; i < kernelSize; ++i)
    {
        vec3 sampleOffset = TBN * samples[i];  // Now in view space
        vec3 sampleViewPos = viewPos + sampleOffset * radius;
        
        // 4. Project sample position to screen space
        vec4 clipPos = projection * vec4(sampleViewPos, 1.0);
        clipPos.xyz /= clipPos.w;  // Perspective divide
        vec2 screenUV = clipPos.xy * 0.5 + 0.5;  // Convert to texture coordinates

        // 5. Get comparison depth in view space
        vec3 sampleWorldPos = texture(gPosition, screenUV).xyz;
        vec3 sampleViewPosActual = vec3(view * vec4(sampleWorldPos, 1.0));
        
        // 6. Range check and accumulate
        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(viewPos.z - sampleViewPosActual.z));
        occlusion += (sampleViewPosActual.z >= sampleViewPos.z + bias ? 1.0 : 0.0) * rangeCheck;
    }
    
    occlusion = 1.0 - (occlusion / kernelSize);
    FragColor = occlusion;
}

#endif
#endif