#ifndef RADIUMENGINE_GIZMO_HPP_
#define RADIUMENGINE_GIZMO_HPP_

#include <vector>

#include <Core/Index/Index.hpp>
#include <Core/Math/LinearAlgebra.hpp>
#include <Engine/Component/Component.hpp>
#include <Engine/RadiumEngine.hpp>
#include <Engine/Renderer/RenderObject/RenderObject.hpp>
#include <Engine/Renderer/RenderObject/RenderObjectManager.hpp>

namespace Ra {
namespace Engine {
class Component;
class RenderObject;
class Camera;
} // namespace Engine
} // namespace Ra

namespace Ra {
namespace Gui {
/// Base class for gizmos, i.e. graphic tools to manipulate a transform.
class Gizmo {
  public:
    /// Used to indicate whether the transform is expressed in world or object space.
    enum Mode {
        LOCAL,
        GLOBAL,
    };

  public:
    RA_CORE_ALIGNED_NEW
    Gizmo( Engine::Component* c, const Core::Transform& worldTo, const Core::Transform& t,
           Mode mode ) :
        m_worldTo( worldTo ),
        m_transform( t ),
        m_comp( c ),
        m_mode( mode ) {}

    virtual ~Gizmo() {
        for ( auto ro : m_renderObjects )
        {
            m_comp->removeRenderObject( ro );
        }
    }

    /// Toggle on/off showing the gizmo.
    void show( bool on ) {
        auto roMgr = Ra::Engine::RadiumEngine::getInstance()->getRenderObjectManager();
        for ( auto ro : m_renderObjects )
        {
            roMgr->getRenderObject( ro )->setVisible( on );
        }
    }

    /// Called every time the underlying transform may have changed.
    virtual void updateTransform( Mode mode, const Core::Transform& worldTo,
                                  const Core::Transform& t ) = 0;

    /// Called when one of the drawables of the gizmo has been selected.
    virtual void selectConstraint( int drawableIndex ) = 0;

    /// Called when the gizmo is first clicked, with the camera parameters and
    /// the initial pixel coordinates.
    virtual void setInitialState( const Engine::Camera& cam, const Core::Vector2& initialXY ) = 0;

    /// Called when the mose movement is recorder with the camera parameters and
    /// the current pixel coordinates.
    virtual Core::Transform mouseMove( const Engine::Camera& cam, const Core::Vector2& nextXY,
                                       bool stepped = false ) = 0;

  protected:
    Core::Transform m_worldTo;                ///< World to local space where the transform lives.
    Core::Transform m_transform;              ///< Transform to be edited.
    Engine::Component* m_comp;                ///< Engine Ui component
    Mode m_mode;                              ///< local or global
    std::vector<Core::Index> m_renderObjects; ///< The RenderObjects used to display the gizmo
};

} // namespace Gui
} // namespace Ra

#endif // RADIUMENGINE_GIZMO_HPP_
