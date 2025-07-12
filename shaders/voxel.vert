#version 450

// Vertex attributes
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in uint inTextureIndex;

// Uniform buffer
layout(binding = 0, std140) uniform UniformBuffer {
    mat4 viewMatrix;
    mat4 projMatrix;
    vec4 lightDirection;
    vec4 viewPosition;
    vec4 fogColor;
    float fogStart;
    float fogEnd;
    float padding[2];
} ubo;

// Output to fragment shader
layout(location = 0) out vec3 fragWorldPos;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec2 fragTexCoord;
layout(location = 3) flat out uint fragTextureIndex;
layout(location = 4) out float fragFogFactor;

void main() {
    // Transform position to clip space
    vec4 worldPos = vec4(inPosition, 1.0);
    vec4 viewPos = ubo.viewMatrix * worldPos;
    gl_Position = ubo.projMatrix * viewPos;

    // Pass data to fragment shader
    fragWorldPos = inPosition;
    fragNormal = inNormal;
    fragTexCoord = inTexCoord;
    fragTextureIndex = inTextureIndex;

    // Calculate fog factor
    float distance = length(viewPos.xyz);
    fragFogFactor = clamp((ubo.fogEnd - distance) / (ubo.fogEnd - ubo.fogStart), 0.0, 1.0);
}