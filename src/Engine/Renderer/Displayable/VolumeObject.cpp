#include <Core/Geometry/Volume.hpp>

#include <Engine/Renderer/Displayable/VolumeObject.hpp>
#include <Engine/Renderer/OpenGL/OpenGL.hpp>

#include <Core/Geometry/MeshPrimitives.hpp>
#include <Core/Types.hpp>

namespace Ra {
namespace Engine {

VolumeObject::VolumeObject( const std::string& name ) :
    Displayable( name ),
    m_tex( TextureParameters() ),
    m_mesh( name + "_internal" ) {

    using Core::Aabb;

    m_tex.m_textureParameters.target = GL_TEXTURE_3D;
    m_tex.m_textureParameters.format = GL_RED;
    m_tex.m_textureParameters.internalFormat = GL_RED;
    m_tex.m_textureParameters.type = GL_FLOAT;
}

VolumeObject::~VolumeObject() {}

void VolumeObject::loadGeometry( Core::Geometry::AbstractVolume* volume ) {
    if ( volume != nullptr && volume->isDense() )
    {

        m_mesh.loadGeometry( Core::Geometry::makeSharpBox( volume->computeAabb() ) );

        // FIXME: assume a given vertex ordering for makeSharpBox. Should be moved in makeSharpBox,
        // however there is currently no rational for the automatic conversion of texture
        // coordinates between TriangleMesh and Mesh

        /*
        Vector3ui( 0, 1, 2 ),    Vector3ui( 0, 2, 3 ),    // Floor
        Vector3ui( 4, 5, 6 ),    Vector3ui( 4, 6, 7 ),    // Ceil
        Vector3ui( 8, 9, 10 ),   Vector3ui( 8, 10, 11 ),  // Left
        Vector3ui( 12, 13, 14 ), Vector3ui( 12, 14, 15 ), // Right
        Vector3ui( 16, 17, 18 ), Vector3ui( 16, 18, 19 ), // Bottom
        Vector3ui( 20, 21, 22 ), Vector3ui( 20, 22, 23 )  // Top
        */

        Core::Vector3Array tex_coords;
        tex_coords.resize( 24 );
        tex_coords.getMap() <<
            // R
            Scalar( 1 ),
            Scalar( 1 ), Scalar( 0 ), Scalar( 0 ),              // Bottom
            Scalar( 1 ), Scalar( 0 ), Scalar( 0 ), Scalar( 1 ), // Top
            Scalar( 1 ), Scalar( 1 ), Scalar( 1 ), Scalar( 1 ), // Right
            Scalar( 0 ), Scalar( 0 ), Scalar( 0 ), Scalar( 0 ), // Left
            Scalar( 1 ), Scalar( 0 ), Scalar( 0 ), Scalar( 1 ), // Floor
            Scalar( 1 ), Scalar( 1 ), Scalar( 0 ), Scalar( 0 ), // Ceil
            // G
            Scalar( 0 ), Scalar( 1 ), Scalar( 1 ), Scalar( 0 ), // Bottom
            Scalar( 0 ), Scalar( 0 ), Scalar( 1 ), Scalar( 1 ), // Top
            Scalar( 1 ), Scalar( 0 ), Scalar( 0 ), Scalar( 1 ), // Right
            Scalar( 0 ), Scalar( 1 ), Scalar( 1 ), Scalar( 0 ), // Left
            Scalar( 0 ), Scalar( 0 ), Scalar( 0 ), Scalar( 0 ), // Floor
            Scalar( 1 ), Scalar( 1 ), Scalar( 1 ), Scalar( 1 ), // Ceil
            // B
            Scalar( 0 ), Scalar( 0 ), Scalar( 0 ), Scalar( 0 ), // Bottom
            Scalar( 1 ), Scalar( 1 ), Scalar( 1 ), Scalar( 1 ), // Top
            Scalar( 0 ), Scalar( 0 ), Scalar( 1 ), Scalar( 1 ), // Right
            Scalar( 0 ), Scalar( 0 ), Scalar( 1 ), Scalar( 1 ), // Left
            Scalar( 0 ), Scalar( 0 ), Scalar( 1 ), Scalar( 1 ), // Floor
            Scalar( 0 ), Scalar( 1 ), Scalar( 1 ), Scalar( 0 ); // Ceil

        m_mesh.addData( Ra::Engine::Mesh::VERTEX_TEXCOORD, tex_coords );

        Core::Geometry::VolumeGrid* grid = static_cast<Core::Geometry::VolumeGrid*>( volume );
        m_volume = std::unique_ptr<Core::Geometry::AbstractVolume>( volume );
        m_tex.m_textureParameters.texels = const_cast<float*>( &( grid->data()[0] ) );

        auto dim = grid->size();

        m_tex.m_textureParameters.width = size_t( dim( 0 ) );
        m_tex.m_textureParameters.height = size_t( dim( 1 ) );
        m_tex.m_textureParameters.depth = size_t( dim( 2 ) );
        m_isDirty = true;
    }
}

void VolumeObject::updateGL() {
    if ( m_isDirty )
    {
        // Check the texture is not too big. In that case, we keep only the first values
        {
            GLint mmax = 2;
            glGetIntegerv( GL_MAX_3D_TEXTURE_SIZE, &mmax );

            if ( m_tex.m_textureParameters.depth > size_t( mmax ) )
            {
                size_t tmp = m_tex.m_textureParameters.depth;
                m_tex.m_textureParameters.depth = size_t( mmax );
                LOG( Ra::Core::Utils::logWARNING ) << "Reducing texture depth from " << tmp
                                                   << " to " << m_tex.m_textureParameters.depth;
            }
        }

        m_mesh.updateGL();
        GL_CHECK_ERROR;
        m_tex.initializeGL();
        GL_CHECK_ERROR;
        m_isDirty = false;
    }
}

void VolumeObject::render() {
    GL_CHECK_ERROR;
    m_mesh.render();
    GL_CHECK_ERROR;
}

} // namespace Engine
} // namespace Ra
