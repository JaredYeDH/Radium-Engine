#include <Engine/Renderer/Material/MaterialConverters.hpp>
#include <Engine/Renderer/Material/RayMarchingMaterial.hpp>
#include <Engine/Renderer/RenderTechnique/RenderTechnique.hpp>
#include <Engine/Renderer/RenderTechnique/ShaderConfigFactory.hpp>
#include <Engine/Renderer/RenderTechnique/ShaderProgram.hpp>

#include <Engine/Renderer/Texture/Texture.hpp>
#include <Engine/Renderer/Texture/TextureManager.hpp>

namespace Ra {
namespace Engine {

static const std::string materialName{"RayMarching"};

RayMarchingMaterial::RayMarchingMaterial( const std::string& name ) :
    Material( name, materialName, Material::MaterialAspect::MAT_OPAQUE ) {}

RayMarchingMaterial::~RayMarchingMaterial() {}

void RayMarchingMaterial::updateGL() {
    if ( !m_isDirty )
    {
        return;
    }

    m_isDirty = false;
}

void RayMarchingMaterial::bind( const ShaderProgram* shader ) {

    shader->setUniform( "material.maxValuePerRay", m_maxValuePerRay );
    shader->setUniform( "material.valuePowCorrection", m_valuePowCorrection );
    shader->setUniform( "material.realOpacityFactor", m_realOpacityFactor );
    shader->setUniform( "material.stepsize", m_stepSize );
    shader->setUniform( "material.uvNormalizationFactor", m_uvNormalizationFactor );
    shader->setUniformTexture( "material.buffer", m_texture );
}

bool RayMarchingMaterial::isTransparent() const {
    // return true if the material is to be used as transparent, false else.
    return true;
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

        []( Ra::Engine::RenderTechnique& rt, bool /*isTransparent*/ ) {
            // Configure the technique to render this object using forward Renderer or any
            // compatible one Main pass (Mandatory) : RayMarching
            auto lightpassconfig =
                Ra::Engine::ShaderConfigurationFactory::getConfiguration( materialName );
            // set one of the following according to the way the material must be used (see
            // RayMarching.frag.glsl)
            rt.setConfiguration( lightpassconfig,
                                 Ra::Engine::RenderTechnique::LIGHTING_TRANSPARENT );
            // rt.setConfiguration( lightpassconfig, Ra::Engine::RenderTechnique::LIGHTING_OPAQUE );
            // rt.setConfiguration( lightpassconfig, Ra::Engine::RenderTechnique::Z_PREPASS );

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
    EngineMaterialConverters::removeMaterialConverter( materialName );
    EngineRenderTechniques::removeDefaultTechnique( materialName );
}

} // namespace Engine
} // namespace Ra
