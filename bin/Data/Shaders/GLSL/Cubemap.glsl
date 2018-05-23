#include "Uniforms.glsl"
#include "Samplers.glsl"
#include "Transform.glsl"

varying vec3 vTexCoord;

void VS()
{
    mat4 modelMatrix = iModelMatrix;
    vec3 worldPos = GetWorldPos(modelMatrix);
    gl_Position = GetClipPos(worldPos);
    gl_Position.z = gl_Position.w;
    vTexCoord = vec3(iPos.x, iPos.y, -iPos.z);
}

void PS()
{
    gl_FragColor = cMatDiffColor * textureCube(sDiffCubeMap, vTexCoord);
}
