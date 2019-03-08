#include <Engine/Component/GeometryComponent.hpp>

#include <iostream>

#include <Core/Asset/FileData.hpp>
#include <Core/Asset/GeometryData.hpp>
#include <Core/Asset/VolumeData.hpp>
#include <Core/Containers/MakeShared.hpp>
#include <Core/Geometry/Normal.hpp>
#include <Core/Utils/Color.hpp>

#include <Engine/Managers/ComponentMessenger/ComponentMessenger.hpp>
#include <Engine/Renderer/RenderObject/RenderObjectManager.hpp>

#include <Engine/Renderer/Displayable/VolumeObject.hpp>
#include <Engine/Renderer/Mesh/Mesh.hpp>

#include <Engine/Renderer/Material/BlinnPhongMaterial.hpp>
#include <Engine/Renderer/Material/Material.hpp>
#include <Engine/Renderer/Material/MaterialConverters.hpp>
#include <Engine/Renderer/Material/RayMarchingMaterial.hpp>

#include <Engine/Renderer/RenderObject/Primitives/DrawPrimitives.hpp>
#include <Engine/Renderer/RenderObject/RenderObject.hpp>
#include <Engine/Renderer/RenderObject/RenderObjectTypes.hpp>

#include <Engine/Renderer/RenderTechnique/RenderTechnique.hpp>
#include <Engine/Renderer/RenderTechnique/ShaderConfigFactory.hpp>
#include <Engine/Renderer/RenderTechnique/ShaderProgram.hpp>
#include <Engine/Renderer/RenderTechnique/ShaderProgramManager.hpp>

using TriangleArray = Ra::Core::VectorArray<Ra::Core::Vector3ui>;

namespace Ra {
namespace Engine {
TriangleMeshComponent::TriangleMeshComponent( const std::string& name, Entity* entity,
                                              const Ra::Core::Asset::GeometryData* data ) :
    Component( name, entity ),
    _displayMesh( nullptr ) {
    generateTriangleMesh( data );
}

//////////// STORE Mesh/TriangleMesh here instead of an index, so don't need to request the ROMgr
/// and no problem with the Displayable -> doesn't affect the API

TriangleMeshComponent::~TriangleMeshComponent() = default;

void TriangleMeshComponent::initialize() {}

void TriangleMeshComponent::generateTriangleMesh( const Ra::Core::Asset::GeometryData* data ) {
    std::string name( m_name );
    name.append( "_" + data->getName() );

    std::string roName = name;

    roName.append( "_RO" );
    std::string meshName = name;
    meshName.append( "_Mesh" );

    std::string matName = name;
    matName.append( "_Mat" );

    m_contentName = data->getName();

    _displayMesh = Ra::Core::make_shared<Mesh>( meshName );

    Ra::Core::Geometry::TriangleMesh mesh;
    const auto T = data->getFrame();
    const Ra::Core::Transform N( ( T.matrix() ).inverse().transpose() );

    mesh.vertices().resize( data->getVerticesSize(), Ra::Core::Vector3::Zero() );

#pragma omp parallel for
    for ( uint i = 0; i < data->getVerticesSize(); ++i )
    {
        mesh.vertices()[i] = T * data->getVertices()[i];
    }

    if ( data->hasNormals() )
    {
        mesh.normals().resize( data->getVerticesSize(), Ra::Core::Vector3::Zero() );
#pragma omp parallel for
        for ( uint i = 0; i < data->getVerticesSize(); ++i )
        {
            mesh.normals()[i] = ( N * data->getNormals()[i] ).normalized();
        }
    }

    const auto& faces = data->getFaces();
    mesh.m_triangles.resize( faces.size(), Ra::Core::Vector3ui::Zero() );
#pragma omp parallel for
    for ( uint i = 0; i < faces.size(); ++i )
    {
        mesh.m_triangles[i] = faces[i].head<3>();
    }

    _displayMesh->loadGeometry( std::move( mesh ) );

    if ( data->hasTangents() )
    {
        _displayMesh->addData( Mesh::VERTEX_TANGENT, data->getTangents() );
    }

    if ( data->hasBiTangents() )
    {
        _displayMesh->addData( Mesh::VERTEX_BITANGENT, data->getBiTangents() );
    }

    if ( data->hasTextureCoordinates() )
    {
        _displayMesh->addData( Mesh::VERTEX_TEXCOORD, data->getTexCoords() );
    }

    if ( data->hasColors() )
    {
        _displayMesh->addData( Mesh::VERTEX_COLOR, data->getColors() );
    }

    // To be discussed: Should not weights be part of the geometry ?
    //        mesh->addData( Mesh::VERTEX_WEIGHTS, meshData.weights );

    // The technique for rendering this component
    RenderTechnique rt;

    bool isTransparent{false};
    if ( data->hasMaterial() )
    {
        const Ra::Core::Asset::MaterialData& loadedMaterial = data->getMaterial();

        // First extract the material from asset
        auto converter = EngineMaterialConverters::getMaterialConverter( loadedMaterial.getType() );
        auto convertedMaterial = converter.second( &loadedMaterial );

        // Second, associate the material to the render technique
        std::shared_ptr<Material> radiumMaterial( convertedMaterial );
        if ( radiumMaterial != nullptr )
        {
            isTransparent = radiumMaterial->isTransparent();
        }
        rt.setMaterial( radiumMaterial );

        // Third, define the technique for rendering this material (here, using the default)
        auto builder = EngineRenderTechniques::getDefaultTechnique( loadedMaterial.getType() );
        builder.second( rt, isTransparent );
    } else
    {
        auto mat =
            Ra::Core::make_shared<BlinnPhongMaterial>( data->getName() + "_DefaultBPMaterial" );
        mat->m_kd = Ra::Core::Utils::Color::Grey();
        mat->m_ks = Ra::Core::Utils::Color::White();
        rt.setMaterial( mat );
        auto builder = EngineRenderTechniques::getDefaultTechnique( "BlinnPhong" );
        builder.second( rt, isTransparent );
    }

    auto ro = RenderObject::createRenderObject( roName, this, RenderObjectType::Geometry,
                                                _displayMesh, rt );
    ro->setTransparent( isTransparent );

    setupIO( m_contentName );
    m_meshIndex = addRenderObject( ro );
}

Ra::Core::Utils::Index TriangleMeshComponent::getRenderObjectIndex() const {
    CORE_ASSERT( _displayMesh != nullptr, "DisplayMesh should exist while component is alive" );
    return m_meshIndex;
}

const Ra::Core::Geometry::TriangleMesh& TriangleMeshComponent::getMesh() const {
    CORE_ASSERT( _displayMesh != nullptr, "DisplayMesh should exist while component is alive" );
    return _displayMesh->getTriangleMesh();
}

void TriangleMeshComponent::setContentName( const std::string& name ) {
    this->m_contentName = name;
}

void TriangleMeshComponent::setupIO( const std::string& id ) {
    CORE_ASSERT( _displayMesh != nullptr, "DisplayMesh should exist while component is alive" );
    ComponentMessenger::CallbackTypes<Ra::Core::Geometry::TriangleMesh>::Getter cbOut =
        std::bind( &TriangleMeshComponent::getMeshOutput, this );
    ComponentMessenger::getInstance()->registerOutput<Ra::Core::Geometry::TriangleMesh>(
        getEntity(), this, id, cbOut );

    ComponentMessenger::CallbackTypes<Ra::Core::Geometry::TriangleMesh>::ReadWrite cbRw =
        std::bind( &TriangleMeshComponent::getMeshRw, this );
    ComponentMessenger::getInstance()->registerReadWrite<Ra::Core::Geometry::TriangleMesh>(
        getEntity(), this, id, cbRw );

    ComponentMessenger::CallbackTypes<Ra::Core::Utils::Index>::Getter roOut =
        std::bind( &TriangleMeshComponent::roIndexRead, this );
    ComponentMessenger::getInstance()->registerOutput<Ra::Core::Utils::Index>( getEntity(), this,
                                                                               id, roOut );

    ComponentMessenger::CallbackTypes<Ra::Core::Vector3Array>::ReadWrite vRW =
        std::bind( &TriangleMeshComponent::getVerticesRw, this );
    ComponentMessenger::getInstance()->registerReadWrite<Ra::Core::Vector3Array>( getEntity(), this,
                                                                                  id + "v", vRW );

    ComponentMessenger::CallbackTypes<Ra::Core::Vector3Array>::ReadWrite nRW =
        std::bind( &TriangleMeshComponent::getNormalsRw, this );
    ComponentMessenger::getInstance()->registerReadWrite<Ra::Core::Vector3Array>( getEntity(), this,
                                                                                  id + "n", nRW );

    ComponentMessenger::CallbackTypes<TriangleArray>::ReadWrite tRW =
        std::bind( &TriangleMeshComponent::getTrianglesRw, this );
    ComponentMessenger::getInstance()->registerReadWrite<TriangleArray>( getEntity(), this,
                                                                         id + "t", tRW );
}

const Ra::Core::Geometry::TriangleMesh* TriangleMeshComponent::getMeshOutput() const {
    return &_displayMesh->getTriangleMesh();
}

Ra::Core::Geometry::TriangleMesh* TriangleMeshComponent::getMeshRw() {
    CORE_ASSERT( _displayMesh != nullptr, "DisplayMesh should exist while component is alive" );
    _displayMesh->setDirty( Mesh::VERTEX_POSITION );
    _displayMesh->setDirty( Mesh::VERTEX_NORMAL );
    _displayMesh->setDirty( Mesh::INDEX );
    return &( _displayMesh->getTriangleMesh() );
}

Ra::Core::Geometry::TriangleMesh::PointAttribHandle::Container*
TriangleMeshComponent::getVerticesRw() {
    CORE_ASSERT( _displayMesh != nullptr, "DisplayMesh should exist while component is alive" );
    _displayMesh->setDirty( Mesh::VERTEX_POSITION );
    return &( _displayMesh->getTriangleMesh().vertices() );
}

Ra::Core::Geometry::TriangleMesh::NormalAttribHandle::Container*
TriangleMeshComponent::getNormalsRw() {
    CORE_ASSERT( _displayMesh != nullptr, "DisplayMesh should exist while component is alive" );
    _displayMesh->setDirty( Mesh::VERTEX_NORMAL );
    return &( _displayMesh->getTriangleMesh().normals() );
}

Ra::Core::VectorArray<Ra::Core::Vector3ui>* TriangleMeshComponent::getTrianglesRw() {
    CORE_ASSERT( _displayMesh != nullptr, "DisplayMesh should exist while component is alive" );
    _displayMesh->setDirty( Mesh::INDEX );
    return &( _displayMesh->getTriangleMesh().m_triangles );
}

const Ra::Core::Utils::Index* TriangleMeshComponent::roIndexRead() const {
    CORE_ASSERT( _displayMesh != nullptr, "DisplayMesh should exist while component is alive" );
    return &m_meshIndex;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////// Volume Component
///////////////////////////////////

VolumeComponent::VolumeComponent( const std::string& name, Entity* entity,
                                  const Ra::Core::Asset::VolumeData* data ) :
    Component( name, entity ),
    _displayVolume( nullptr ) {
    generateVolumeRender( data );
}

VolumeComponent::~VolumeComponent() = default;

void VolumeComponent::initialize() {}

void VolumeComponent::generateVolumeRender( const Ra::Core::Asset::VolumeData* data ) {
    std::string name( m_name );
    name.append( "_" + data->getName() );

    std::string roName = name;

    roName.append( "_RO" );
    std::string meshName = name;
    meshName.append( "_Mesh" );

    std::string matName = name;
    matName.append( "_Mat" );

    m_contentName = name + "_DAT_" + data->getName();

    _displayVolume = Ra::Core::make_shared<VolumeObject>( meshName );
    _displayVolume->loadGeometry( data->volume );

    // The technique for rendering this component
    RenderTechnique rt;

    auto mat = Ra::Core::make_shared<RayMarchingMaterial>( data->getName() + "_DefaultBPMaterial" );
    mat->setTexture( const_cast<Texture*>( &( _displayVolume->getDataTexture() ) ) );
    // compute uv coordinates normalization factor
    {
        Core::Vector3 s = _displayVolume->getVolume().computeAabb().sizes();
        mat->m_uvNormalizationFactor = s / s.minCoeff();
    }
    rt.setMaterial( mat );
    // Do not set the material transparent if it is not ... ask wether material is transparent.
    bool isTransparent{mat->isTransparent()};
    auto builder = EngineRenderTechniques::getDefaultTechnique( "RayMarching" );
    builder.second( rt, isTransparent );

    auto ro = RenderObject::createRenderObject( roName, this, RenderObjectType::Geometry,
                                                _displayVolume, rt );
    ro->setTransparent( isTransparent );

    setupIO( m_contentName );
    m_volumeIndex = addRenderObject( ro );
}

Ra::Core::Utils::Index VolumeComponent::getRenderObjectIndex() const {
    CORE_ASSERT( _displayVolume != nullptr, "DisplayMesh should exist while component is alive" );
    return m_volumeIndex;
}

void VolumeComponent::setContentName( const std::string& name ) {
    this->m_contentName = name;
}

void VolumeComponent::setupIO( const std::string& id ) {
    CORE_ASSERT( _displayVolume != nullptr, "DisplayMesh should exist while component is alive" );
    ComponentMessenger::CallbackTypes<Ra::Core::Geometry::AbstractVolume>::Getter cbOut =
        std::bind( &VolumeComponent::getVolumeOutput, this );
    ComponentMessenger::getInstance()->registerOutput<Ra::Core::Geometry::AbstractVolume>(
        getEntity(), this, id, cbOut );

    ComponentMessenger::CallbackTypes<Ra::Core::Geometry::AbstractVolume>::ReadWrite cbRw =
        std::bind( &VolumeComponent::getVolumeRw, this );
    ComponentMessenger::getInstance()->registerReadWrite<Ra::Core::Geometry::AbstractVolume>(
        getEntity(), this, id, cbRw );

    ComponentMessenger::CallbackTypes<Ra::Core::Utils::Index>::Getter roOut =
        std::bind( &VolumeComponent::roIndexRead, this );
    ComponentMessenger::getInstance()->registerOutput<Ra::Core::Utils::Index>( getEntity(), this,
                                                                               id, roOut );
}

const Ra::Core::Utils::Index* VolumeComponent::roIndexRead() const {
    CORE_ASSERT( _displayVolume != nullptr, "DisplayMesh should exist while component is alive" );
    return &m_volumeIndex;
}

const Ra::Core::Geometry::AbstractVolume* VolumeComponent::getVolumeOutput() const {
    return &_displayVolume->getVolume();
}

Ra::Core::Geometry::AbstractVolume* VolumeComponent::getVolumeRw() {
    return &_displayVolume->getVolume();
}

} // namespace Engine
} // namespace Ra
