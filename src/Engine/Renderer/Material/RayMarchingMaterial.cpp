#include <Engine/Renderer/Material/MaterialConverters.hpp>
#include <Engine/Renderer/Material/RayMarchingMaterial.hpp>
#include <Engine/Renderer/RenderTechnique/RenderTechnique.hpp>
#include <Engine/Renderer/RenderTechnique/ShaderConfigFactory.hpp>
#include <Engine/Renderer/RenderTechnique/ShaderProgram.hpp>

#include <Engine/Renderer/Texture/Texture.hpp>
#include <Engine/Renderer/Texture/TextureManager.hpp>

namespace Ra {
namespace Engine {

RayMarchingMaterial::RayMarchingMaterial( const std::string& name ) :
    Material( name, Material::MaterialAspect::MAT_OPAQUE ) {}

RayMarchingMaterial::~RayMarchingMaterial() {}

void RayMarchingMaterial::updateGL() {
    if ( !m_isDirty )
    {
        return;
    }

    m_isDirty = false;
}

const std::string RayMarchingMaterial::getShaderInclude() const {
    return "RayMarching";
}

void RayMarchingMaterial::bind( const ShaderProgram* shader ) {

    shader->setUniformTexture( "material.buffer", m_texture );
}

bool RayMarchingMaterial::isTransparent() const {
    return false;
}

void RayMarchingMaterial::registerMaterial() {
    // registering re-usable shaders
    Ra::Engine::ShaderConfiguration lpconfig(
        "RayMarching", "Shaders/Materials/RayMarching/RayMarching.vert.glsl",
        "Shaders/Materials/RayMarching/RayMarching.frag.glsl" );
    Ra::Engine::ShaderConfigurationFactory::addConfiguration( lpconfig );

    //    Ra::Engine::ShaderConfiguration zprepassconfig(
    //        "ZprepassRayMarching", "Shaders/Materials/RayMarching/RayMarching.vert.glsl",
    //        "Shaders/Materials/RayMarching/DepthAmbientRayMarching.frag.glsl" );
    //    Ra::Engine::ShaderConfigurationFactory::addConfiguration( zprepassconfig );

    //    Ra::Engine::ShaderConfiguration transparentpassconfig(
    //        "LitOITRayMarching", "Shaders/Materials/RayMarching/RayMarching.vert.glsl",
    //        "Shaders/Materials/RayMarching/LitOITRayMarching.frag.glsl" );
    //    Ra::Engine::ShaderConfigurationFactory::addConfiguration( transparentpassconfig );

    // Registering technique
    Ra::Engine::EngineRenderTechniques::registerDefaultTechnique(
        "RayMarching",

        []( Ra::Engine::RenderTechnique& rt, bool isTransparent ) {
            // Configure the technique to render this object using forward Renderer or any
            // compatible one Main pass (Mandatory) : RayMarching
            auto lightpassconfig =
                Ra::Engine::ShaderConfigurationFactory::getConfiguration( "RayMarching" );
            //rt.setConfiguration( lightpassconfig, Ra::Engine::RenderTechnique::LIGHTING_OPAQUE );
            rt.setConfiguration( lightpassconfig, Ra::Engine::RenderTechnique::Z_PREPASS );

            //            // Z prepass (Recommended) : DepthAmbiantPass
            //            auto zprepassconfig =
            //                Ra::Engine::ShaderConfigurationFactory::getConfiguration(
            //                "ZprepassRayMarching" );
            //            rt.setConfiguration( zprepassconfig,
            //            Ra::Engine::RenderTechnique::Z_PREPASS );
            //            // Transparent pass (0ptional) : If Transparent ... add LitOIT
            //            if ( isTransparent )
            //            {
            //                auto transparentpassconfig =
            //                    Ra::Engine::ShaderConfigurationFactory::getConfiguration(
            //                    "LitOITRayMarching" );
            //                rt.setConfiguration( transparentpassconfig,
            //                                     Ra::Engine::RenderTechnique::LIGHTING_TRANSPARENT
            //                                     );
            //            }
        } );
}

void RayMarchingMaterial::unregisterMaterial() {
    EngineMaterialConverters::removeMaterialConverter( "RayMarching" );
    EngineRenderTechniques::removeDefaultTechnique( "RayMarching" );
}

} // namespace Engine
} // namespace Ra
