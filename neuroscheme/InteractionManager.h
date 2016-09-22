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
#ifndef __NEUROSCHEME__INTERACTION_MANAGER__
#define __NEUROSCHEME__INTERACTION_MANAGER__

#include "reps/SelectableItem.h"
#include <QAbstractGraphicsShapeItem>
#include <QGraphicsSceneMouseEvent>
#include <QMenu>
#include <QPen>
#include <iostream> // Borrar
namespace neuroscheme
{
  class InteractionManager
  {
  public:

    static void hoverEnterEvent( QAbstractGraphicsShapeItem* item,
                                 QGraphicsSceneHoverEvent* event );

    static void hoverLeaveEvent( QAbstractGraphicsShapeItem* item,
                                 QGraphicsSceneHoverEvent* event );

    static void contextMenuEvent( QAbstractGraphicsShapeItem* item,
                                  QGraphicsSceneContextMenuEvent* event );

    static void mousePressEvent( QAbstractGraphicsShapeItem* item,
                                 QGraphicsSceneMouseEvent* event );

    protected:

    static QPen _selectedPen;
    static QPen _unselectedPen;
    static QPen _hoverSelectedPen;
    static QPen _hoverUnselectedPen;

    static QMenu* _contextMenu;


  };
}

#endif