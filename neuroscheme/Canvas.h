#ifndef __NEUROSCHEME__CANVAS__
#define __NEUROSCHEME__CANVAS__

#include <QWidget>
#include <QGraphicsView>
#include <QMouseEvent>

namespace neuroscheme
{

  class GraphicsView : public QGraphicsView
  {

    Q_OBJECT;

  public:
    GraphicsView( QWidget* parent = 0 );

  protected:
    virtual void wheelEvent( QWheelEvent* event_ );

  }; // class GraphicsView

  class GraphicsScene : public QGraphicsScene
  {
  }; // class GraphicsScene


  class Canvas : public QWidget
  {
  public:
    Canvas( QWidget* parent = 0 );
    ~Canvas( void );

    const GraphicsScene& scene( void ) const;
    GraphicsScene& scene( void );

    const GraphicsView& view( void ) const;
    GraphicsView& view( void );

  protected:
    GraphicsView* _graphicsView;
    GraphicsScene* _graphicsScene;

  }; // class Canvas



} // namespace neuroscheme

#endif // __NEUROSCHEME__CANVAS__