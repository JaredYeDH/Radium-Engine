#ifndef RADIUMENGINE_RAYMARCHINGMATERIAL_HPP
#define RADIUMENGINE_RAYMARCHINGMATERIAL_HPP

#include <Engine/RaEngine.hpp>

#include <map>
#include <string>

#include <Core/Utils/Color.hpp>
#include <Engine/Renderer/Material/Material.hpp>
#include <Engine/Renderer/Texture/Texture.hpp>

namespace Ra {
namespace Core {
namespace Asset {
class MaterialData;
}
} // namespace Core
namespace Engine {

class ShaderProgram;

/**
 * Implementation of a simple RayMarching Material for Volume Objects.
 */
class RA_ENGINE_API RayMarchingMaterial final : public Material {

  public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
    /**
     * Construct a named Blinn-Phongmaterial
     * @param name The name of the material
     */
    explicit RayMarchingMaterial( const std::string& name );
    /**
     * Destructor.
     * @note The material does not have ownership on its texture. This destructor do not delete the
     * associated textures.
     */
    ~RayMarchingMaterial() override;

    /**
     * Get the basename of the glsl source file to include if one want to build composite shaders
     * that use this material.
     * @return The basename (without extension .frag.glsl or .vert.glsl) of the glsl source file.
     */
    const std::string getShaderInclude() const override;

    void updateGL() override;
    void bind( const ShaderProgram* shader ) override;
    bool isTransparent() const override;

    /**
     * Add an already existing texture to control the specified BSDF parameter.
     * @param semantic The texture semantic
     * @param texture  The texture to use
     */
    inline void setTexture( Texture* texture );

    /**
     * Get the texture associated to the given semantic.
     * @param semantic
     * @return
     */
    inline Texture* getTexture() const;

    /**
     * Register the material in the material library.
     * After registration, the material could be instantiated by any Radium system, renderer,
     * plugin, ...
     */
    static void registerMaterial();

    /**
     * Remove the material from the material library.
     * After removal, the material is no more available, ...
     */
    static void unregisterMaterial();

  public:
  private:
    Texture* m_texture;
};

} // namespace Engine
} // namespace Ra

#include <Engine/Renderer/Material/RayMarchingMaterial.inl>
#endif // RADIUMENGINE_RAYMARCHINGMATERIAL_HPP
