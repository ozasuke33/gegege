#include "../../../include/gegege/otsukimi/util.hpp"

#include "../../../include/gegege/otsukimi/renderer.hpp"

#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/ext/matrix_clip_space.hpp>

namespace gegege::otsukimi {

extern Renderer* gRenderer;

int getMouseCoordinateToScreenCoordinateX(int mouseX)
{
    float NdcX = 2.0f * mouseX / gRenderer->mScreenWidth - 1.0f;
    glm::mat4 ortho = glm::ortho(float(-gRenderer->mScreenWidth) / 2.0f, float(gRenderer->mScreenWidth) / 2.0f, float(-gRenderer->mScreenHeight) / 2.0f, float(gRenderer->mScreenHeight) / 2.0f);
    float scale = std::min(float(gRenderer->mScreenWidth) / gRenderer->mTargetOffscreenWidth, float(gRenderer->mScreenHeight) / gRenderer->mTargetOffscreenHeight);
    glm::vec4 orthoCoords = glm::inverse(ortho) * glm::vec4(NdcX / scale, 0.0f, 0.0f, 1.0f);

    return orthoCoords.x;
}

int getMouseCoordinateToScreenCoordinateY(int mouseY)
{
    float NdcY = 1.0f - 2.0f * mouseY / gRenderer->mScreenHeight;
    glm::mat4 ortho = glm::ortho(float(-gRenderer->mScreenWidth) / 2.0f, float(gRenderer->mScreenWidth) / 2.0f, float(-gRenderer->mScreenHeight) / 2.0f, float(gRenderer->mScreenHeight) / 2.0f);
    float scale = std::min(float(gRenderer->mScreenWidth) / gRenderer->mTargetOffscreenWidth, float(gRenderer->mScreenHeight) / gRenderer->mTargetOffscreenHeight);
    glm::vec4 orthoCoords = glm::inverse(ortho) * glm::vec4(0.0f, NdcY / scale, 0.0f, 1.0f);

    return orthoCoords.y;
}

}
