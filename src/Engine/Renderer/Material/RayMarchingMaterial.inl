#include <Engine/Renderer/Material/RayMarchingMaterial.hpp>

namespace Ra {
namespace Engine {

// Add a texture as material parameter from an already existing Radium Texture
inline void RayMarchingMaterial::setTexture( Texture* texture ) {
    m_texture = texture;
}

inline Texture* RayMarchingMaterial::getTexture() const {
    return m_texture;
}
} // namespace Engine
} // namespace Ra
