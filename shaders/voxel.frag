#version 450

// Input from vertex shader
layout(location = 0) in vec3 fragWorldPos;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec2 fragTexCoord;
layout(location = 3) flat in uint fragTextureIndex;
layout(location = 4) in float fragFogFactor;

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

// Texture array and sampler
layout(binding = 1) uniform texture2DArray texArray;
layout(binding = 2) uniform sampler texSampler;

// Output
layout(location = 0) out vec4 outColor;

void main() {
    // Sample texture from array
    vec4 texColor = texture(sampler2DArray(texArray, texSampler), vec3(fragTexCoord, float(fragTextureIndex)));

    // Discard transparent pixels
    if (texColor.a < 0.5) {
        discard;
    }

    // Calculate lighting
    vec3 normal = normalize(fragNormal);
    vec3 lightDir = normalize(-ubo.lightDirection.xyz);

    // Ambient lighting
    float ambient = 0.4;

    // Diffuse lighting
    float diffuse = max(dot(normal, lightDir), 0.0);

    // Add some extra brightness to top faces
    if (normal.y > 0.9) {
        diffuse += 0.1;
    }

    // Calculate shadow for bottom faces
    float shadow = 1.0;
    if (normal.y < -0.9) {
        shadow = 0.5;
    }

    // Combine lighting
    vec3 lighting = vec3(ambient + diffuse * 0.6) * shadow;
    vec3 finalColor = texColor.rgb * lighting;

    // Apply fog
    finalColor = mix(ubo.fogColor.rgb, finalColor, fragFogFactor);

    // Output final color
    outColor = vec4(finalColor, texColor.a);
}