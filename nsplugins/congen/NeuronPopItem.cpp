/*
 * Copyright (c) 2016 GMRV/URJC/UPM.
 *
 * Authors: Pablo Toharia <pablo.toharia@upm.es>
 *
 * This file is part of NeuroScheme
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 3.0 as published
 * by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */
#include "NeuronPopItem.h"
#include <nslib/reps/RingItem.h>
#include <QPen>

#define POSX0 1.0f                    // Precomputed values for cos(0)
#define POSY0 0.0f                    // Precomputed values for sin(0)

#define POSX1 0.5000007660251953f     // Precomputed values for cos(60)
#define POSY1 0.8660249615191342f     // Precomputed values for sin(60)

#define POSX2 -0.49999846794843594f   // Precomputed values for cos(120)
#define POSY2 0.8660262883130146f     // Precomputed values for sin(120)

#define POSX3 -0.9999999999964793f    // Precomputed values for cos(180)
#define POSY3 0.00000265358979335273f // Precomputed values for sin(180)

#define POSX4 -0.5000030640984338f    // Precomputed values for cos(240)
#define POSY4 -0.8660236347191557f    // Precomputed values for sin(240)

#define POSX5 0.49999616986815576f    // Precomputed values for cos(300)
#define POSY5 -0.8660276151007971f    // Precomputed values for sin(300)

namespace nslib
{
  namespace congen
  {
    NeuronPopItem::NeuronPopItem( const NeuronPopRep& neuronRep,
                                  unsigned int size,
                                  bool interactive_ )
    {
      setInteractive( interactive_ );
      if ( interactive_ )
      {
        this->setAcceptHoverEvents( true );
      }

      int itemSize = ceil( float( size ) / 2.0f );
      this->setRect ( -itemSize, -itemSize,
                      itemSize * 2 , itemSize * 2 );
      this->setPen( QPen( Qt::NoPen ));

      const Color& bgColor = neuronRep.getProperty( "color" ).value< Color >( );

      auto circleItem = new QGraphicsEllipseItem( // this 
        );
      auto circleItemSize = roundf( size * 0.75f );
      circleItem->setRect( - int( circleItemSize ) / 2,
                           - int( circleItemSize ) / 2,
                           circleItemSize, circleItemSize );
      circleItem->setPen( Qt::NoPen );
      circleItem->setBrush( QBrush( bgColor ));

      QPainterPath path_;
      QPolygon poly;

      /*int size_2 = roundf( size * 0.5f );
      int sizeTopBottomX = roundf( size_2 * 0.5f );
      int sizeTopBottomY = roundf( size_2 * 0.7f );
      int sizeMiddle = roundf( size_2 * 0.9f );
      QPoint pTL ( -sizeTopBottomX, -sizeTopBottomY );
      QPoint pTR (  sizeTopBottomX, -sizeTopBottomY );
      QPoint pBR (  sizeTopBottomX,  sizeTopBottomY );
      QPoint pBL ( -sizeTopBottomX,  sizeTopBottomY );

      QPoint pMR (  sizeMiddle,  0           );
      QPoint pML ( -sizeMiddle,  0           );

      poly << pTL << pTR << pMR << pBR << pBL << pML;*/

      float size_2 = roundf( size * 0.5f );


      poly << QPoint(
        ( size_2 * POSX0 ),
        ( size_2 * POSY0 )
      );

      poly << QPoint(
        ( size_2 * POSX1 ),
        ( size_2 * POSY1 )
      );

      poly << QPoint(
        ( size_2 * POSX2 ),
        ( size_2 * POSY2 )
      );

      poly << QPoint(
        ( size_2 * POSX3 ),
        ( size_2 * POSY3 )
      );

      poly << QPoint(
        ( size_2 * POSX4 ),
        ( size_2 * POSY4 )
      );

      poly << QPoint(
        ( size_2 * POSX5 ),
        ( size_2 * POSY5 )
      );

      // QPoint pUL (-int(size)/2, -int(size)/24);
      // QPoint pUM (           0, -int(size)/6);
      // QPoint pUR ( int(size)/2, -int(size)/24);

      // QPoint pLR ( int(size)/2, +int(size)/24);
      // QPoint pLM (           0, +int(size)/6);
      // QPoint pLL (-int(size)/2, +int(size)/24);

      // poly << pLR << pLM
      //      << pLL
      //      << pUL //<< pUM
      //      << pUR;

      path_.addPolygon( poly );
      path_.closeSubpath(  );

      auto icon = new QGraphicsPathItem( this );
      icon->setPath( path_ );
      icon->setPen( Qt::NoPen );
      icon->setBrush( bgColor ); //QBrush( QColor( 114, 188, 196 )));
      //icon->setBrush( QBrush( baseColor ));

      // auto lineContainerWidth = roundf( circleItemSize * .9f );
      // auto lineContainerHeight = roundf( circleItemSize * .1f );
      // auto lineContainer = new QGraphicsRectItem(
      //   roundf( - int( lineContainerWidth ) * .5f ),
      //   roundf( - int( lineContainerHeight ) * .5f ),
      //   lineContainerWidth,
      //   lineContainerHeight );

      // lineContainer->setPen( Qt::NoPen );
      // lineContainer->setBrush( QBrush( QColor( 255, 255, 255 )));
      // // lineContainer->setParentItem( this );

      //auto linePadding = roundf( lineContainerWidth * 0.01f );
      // auto lineWidth = lineContainerWidth; //roundf(  circleItemSize * .85f );
      // auto lineHeight = lineContainerHeight; //roundf( circleItemSize * .09f );
      auto line = new QGraphicsRectItem(
        POSX4,
        POSY4 - size_2 * 0.03,
        roundf( ( POSX5 - POSX4) *
                neuronRep.getProperty( "line perc" ).value< float >( )),
        size_2 * 0.06 );

        // roundf( - int( lineWidth ) * .5f ) + linePadding,
        // roundf( - int( lineHeight ) * .5f ) + linePadding,
        // roundf( lineWidth *
        //         neuronRep.getProperty( "line perc" ).value< float >( )) -
        // 2 * linePadding,
        // lineHeight - 2 * linePadding);

      line->setPen( Qt::NoPen );
      line->setBrush( QBrush( QColor( 180, 70, 70 )));
      line->setParentItem( this );

      this->_parentRep = &( const_cast< NeuronPopRep& >( neuronRep ));
    }

  } // namespace congen
} // namespace nslib
