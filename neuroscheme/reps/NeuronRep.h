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
#ifndef __NEUROSCHEME__NEURON_REP__
#define __NEUROSCHEME__NEURON_REP__

#include "QGraphicsItemRepresentation.h"
#include <shift/shift.h>
// #include <shift_Ring.h>
#include <shift_NeuronRep.h>

namespace neuroscheme
{

  // class Ring
  //   : public shiftgen::Ring
  // {

  // public:
  //   Ring( void );
  //   Ring( const Ring& other );
  //   virtual ~Ring( void ) {}

  // };

  // using Ring = shiftgen::Ring;
//  typedef std::vector< Ring > Rings;

  class NeuronRep
    : public shiftgen::NeuronRep
    , public QGraphicsItemRepresentation
  {
  public:

    // typedef shiftgen::NeuronRep::Rings Rings;

    NeuronRep( void );
    NeuronRep( const NeuronRep& );
    virtual ~NeuronRep( void ) {}
    QGraphicsItem* item( bool create = true );

  };


} // namespace neuroscheme

#endif
