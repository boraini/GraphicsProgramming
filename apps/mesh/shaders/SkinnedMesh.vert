#version 300 es

in vec3 position;
in vec3 normal;
in vec2 uv;
in ivec4 bone;
in vec4 influence;

out vec2 TexCoord;
out vec3 worldNormal;
out vec3 weightColor;

const int MAX_BONES = 200;

uniform mat4 projectionMatrix;
uniform mat4 cameraInverseMatrix;
uniform mat4 objectMatrix;
uniform mat4 boneMatrices[MAX_BONES];

void main()
{

    mat4 gWVP = projectionMatrix * cameraInverseMatrix * objectMatrix;

    mat4 BoneTransform = boneMatrices[bone[0]] * influence[0];
    BoneTransform     += boneMatrices[bone[1]] * influence[1];
    BoneTransform     += boneMatrices[bone[2]] * influence[2];
    BoneTransform     += boneMatrices[bone[3]] * influence[3];

    vec4 PosL = BoneTransform * vec4(position, 1.0);
    gl_Position = gWVP * PosL;
    TexCoord = uv;
    worldNormal = mat3(gWVP * BoneTransform) * normal;
    weightColor = vec3(0.5);
}