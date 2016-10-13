#include "Canvas.h"
#include "PaneManager.h"
#include <QHBoxLayout>

namespace neuroscheme
{

  GraphicsView::GraphicsView( QWidget* parent_ )
    : QGraphicsView( parent_ )
  {
  }

  void GraphicsView::wheelEvent( QWheelEvent* event_ )
  {

    setTransformationAnchor( QGraphicsView::AnchorUnderMouse );

    // Scale the view / do the zoom
    double scaleFactor = 1.05;
    int delta = event_->angleDelta( ).y( );

    if ( delta > 0 )
    {
      // Zoom in
      scale(scaleFactor, scaleFactor);
    }
    else if ( delta < 0 )
    {
      // Zooming out
      this->scale(1.0 / scaleFactor, 1.0 / scaleFactor);
    }

    // Don't call superclass handler here
    // as wheel is normally used for moving scrollbars
  }

  Canvas::Canvas( QWidget * parent_ )
    : _graphicsView( new GraphicsView( parent_ )),
      _graphicsScene( new GraphicsScene )
  {
    _graphicsView->setScene(_graphicsScene);
    _graphicsView->setInteractive( true );
    _graphicsView->setMouseTracking( true );
    _graphicsView->viewport( )->setMouseTracking( true );
    _graphicsView->setRenderHints( QPainter::Antialiasing |
                                   QPainter::HighQualityAntialiasing );
    _graphicsView->setDragMode( QGraphicsView::ScrollHandDrag );
    this->setSizePolicy( QSizePolicy::MinimumExpanding,
                         QSizePolicy::MinimumExpanding );

    QHBoxLayout* layout_ = new QHBoxLayout;
    layout_->addWidget( _graphicsView );
    this->setLayout( layout_ );

    this->leaveEvent( nullptr );
  }

  Canvas::~Canvas( void )
  {
    if ( _graphicsView ) delete _graphicsView;
    if ( _graphicsScene ) delete _graphicsScene;
  }

  const GraphicsScene& Canvas::scene( void ) const
  {
    return *_graphicsScene;
  }

  GraphicsScene& Canvas::scene( void )
  {
    return *_graphicsScene;
  }

  const GraphicsView& Canvas::view( void ) const
  {
    return *_graphicsView;
  }

  GraphicsView& Canvas::view( void )
  {
    return *_graphicsView;
  }

  void Canvas::enterEvent( QEvent*  )
  {
    neuroscheme::PaneManager::activePane( this );
    this->setObjectName("pane");
    this->setStyleSheet("#pane { border: 3px dotted rgba( 0,0,0,15%); }");

  }

  void Canvas::leaveEvent( QEvent*  )
  {
    this->setObjectName("pane");
    this->setStyleSheet("#pane { border: 3px solid rgba( 0,0,0,0%); }");
  }

  const Layouts& Canvas::layouts( void ) const
  {
    return _layouts;
  }
  Layouts& Canvas::layouts( void )
  {
    return _layouts;
  }

} // namespace neuroscheme
