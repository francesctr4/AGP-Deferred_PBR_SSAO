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

layout(binding = 0) uniform sampler2D gPosition;
layout(binding = 1) uniform sampler2D gNormal;
layout(binding = 2) uniform sampler2D texNoise;

uniform vec3 samples[64];

// Parameters
uniform int kernelSize;
uniform float radius;
uniform float bias;
uniform float power;

uniform float SCREEN_WIDTH;
uniform float SCREEN_HEIGHT;

uniform mat4 view;
uniform mat4 projection;

void main() {
    // Tile noise texture
    vec2 noiseScale = vec2(SCREEN_WIDTH / 4.0, SCREEN_HEIGHT / 4.0);

    // Get world-space position and normal from G-buffer
    vec3 fragPosWorld = texture(gPosition, TexCoords).xyz;
    vec3 normalWorld = normalize(texture(gNormal, TexCoords).rgb);

    // Generate random tangent from noise texture
    vec3 randomVec = normalize(texture(texNoise, TexCoords * noiseScale).xyz);

    // Create TBN matrix (tangent -> world space)
    vec3 tangent = normalize(randomVec - normalWorld * dot(randomVec, normalWorld));
    vec3 bitangent = cross(normalWorld, tangent);
    mat3 TBN = mat3(tangent, bitangent, normalWorld);

    // Calculate occlusion
    float occlusion = 0.0;
    for (int i = 0; i < kernelSize; ++i) {
        // Transform sample to world space
        vec3 sampleOffset = TBN * samples[i]; 
        vec3 samplePosWorld = fragPosWorld + sampleOffset * radius;

        // Project sample position to view space and clip space
        vec4 samplePosView = view * vec4(samplePosWorld, 1.0);
        vec4 clipPos = projection * samplePosView;
        clipPos.xyz /= clipPos.w; // Perspective divide
        vec3 ndc = clipPos.xyz;

        // Convert to UV [0, 1]
        vec2 sampleUV = ndc.xy * 0.5 + 0.5;

        // Get sampled world position from G-buffer
        vec3 sampledPosWorld = texture(gPosition, sampleUV).xyz;

        // Transform sampled position to view space
        vec4 sampledPosView = view * vec4(sampledPosWorld, 1.0);
        float sampledDepth = sampledPosView.z / sampledPosView.w;

        // Range check and accumulate occlusion
        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(samplePosView.z - sampledDepth));
        occlusion += (sampledDepth >= samplePosView.z + bias ? 1.0 : 0.0) * rangeCheck;
    }

    occlusion = 1.0 - (occlusion / kernelSize);
    FragColor = pow(occlusion, power);
}

#endif
#endif