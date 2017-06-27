/*
 * Copyright (c) 2016 GMRV/URJC/UPM.
 *
 * Authors: Juan Pedro Brito <juanpedro.brito@upm.es>
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
#include "EntityEditWidget.h"
#include "DataManager.h"
#include "PaneManager.h"
#include "RepresentationCreatorManager.h"

#include <QtGui>
#include <QPushButton>
#include <QGridLayout>
#include <QInputDialog>

#include <QFileDialog>
#include <QFontDialog>
#include <QColorDialog>
#include <QMessageBox>

EntityEditWidget::EntityEditWidget(
  shift::Entity* entity, TEntityEditWidgetAction action, QWidget *parent )
  : QWidget( parent )
  , _entity( nullptr )
  , _action( action )
{
  QGridLayout* layout = new QGridLayout;
  layout->setColumnStretch( 1, 1 );
  layout->setColumnMinimumWidth( 1, 150 );

  unsigned int element = 0;

  if ( entity )
  {
    for ( const auto& propPair : entity->properties( ))
    {
      const auto prop = propPair.first;
      auto caster = fires::PropertyManager::getPropertyCaster( prop );
      if ( caster )
      {
        auto label = new QLabel(
          QString::fromStdString(
            fires::PropertyGIDsManager::getPropertyLabel( prop )));

        layout->addWidget( label, element, 0 );

        const auto& categories = caster->categories( );

        TWidgetType widgetType;
        QWidget* widget;
        if ( categories.size( ) > 0 )
        {
          widgetType = TWidgetType::COMBO;
          auto comboBoxWidget = new QComboBox;
          widget = comboBoxWidget;
          auto currentCategory = caster->toString( propPair.second );
          unsigned int index = 0;
          for ( const auto& category : categories )
            comboBoxWidget->addItem( QString::fromStdString( category ));
          for ( const auto& category : categories )
          {
            if ( category != currentCategory )
              ++index;
            else
              break;
          }
          comboBoxWidget->setCurrentIndex( index );
        }
        else
        {
          widgetType = TWidgetType::LINE_EDIT;
          auto lineEditwidget = new QLineEdit;
          widget = lineEditwidget;
          lineEditwidget->setText( QString::fromStdString(
                                     caster->toString(propPair.second )));
        }
        layout->addWidget( widget, element, 1 );
        ++element;


        _entityParamCont.push_back(
          std::make_tuple( widgetType, label, widget ));

      }
    }
  }

  QPushButton* cancelButton = new QPushButton( tr( "Cancel" ));
  layout->addWidget( cancelButton, element, 0 );

  QPushButton* validationButton = new QPushButton(tr( "Save" ));
  layout->addWidget( validationButton, element, 1 );

  connect(cancelButton, SIGNAL( clicked( )), this, SLOT( cancelDialog( )));
  connect(validationButton, SIGNAL( clicked( )),
          this, SLOT( validateDialog( )));

  setLayout( layout );
  setWindowTitle( tr( "Edit entity" ));

  _entity = entity;
}

void EntityEditWidget::validateDialog( void )
{
  auto origEntity = _entity;

  if ( _action == DUPLICATE_ENTITY || _action == NEW_ENTITY )
    _entity = _entity->create( );

  for ( const auto& p : _entity->properties( ))
    std::cout << fires::PropertyGIDsManager::getPropertyLabel( p.first ) << " ";
  std::cout << std::endl;

  assert ( _entity );
  for ( const auto& entityParam: _entityParamCont )
  {
    const auto& editType = std::get< TEditTuple::WIDGET_TYPE >( entityParam );
    const auto& labelWidget = std::get< TEditTuple::LABEL >( entityParam );
    const auto& widget = std::get< TEditTuple::WIDGET >( entityParam );
    QString paramString;
    if ( editType ==  TWidgetType::COMBO )
    {
      auto comboWidget = dynamic_cast< QComboBox* >( widget );
      paramString = comboWidget->currentText( );
    }
    else if ( editType == TWidgetType::LINE_EDIT )
    {
      auto lineEditwidget = dynamic_cast< QLineEdit* >( widget );
      paramString = lineEditwidget->text( );
    }
    else
      assert( false );

    const auto& label = labelWidget->text( ).toStdString( );
    auto caster = fires::PropertyManager::getPropertyCaster( label );
    if ( !_entity->hasProperty( label ))
    {
      auto prop = origEntity ->getProperty( label );
      _entity->registerProperty( label, prop );
    }
    auto& prop = _entity->getProperty( label );
    assert ( caster );
    caster->fromString( prop, paramString.toStdString( ));

  }
  this->hide( );

  if ( _action == DUPLICATE_ENTITY || _action == NEW_ENTITY )
  {
    nslib::DataManager::entities( ).add( _entity );
    nslib::PaneManager::activePane( )->entities( ).add( _entity );
    nslib::PaneManager::activePane( )->refreshProperties(
      nslib::PaneManager::activePane( )->entities( ));
    nslib::PaneManager::activePane( )->resizeEvent( nullptr );
  }
  if ( _action == EDIT_ENTITY )
  {
    for ( auto repPair : nslib::RepresentationCreatorManager::repsToEntities( ))
    {
      shift::Representation* rep = repPair.first;
      delete rep;
    }
    nslib::RepresentationCreatorManager::clearEntitiesToReps( );
    for ( auto pane : nslib::PaneManager::panes( ))
    {
      pane->reps( ).clear( );
      pane->resizeEvent( nullptr );
    }
  }
}

void EntityEditWidget::cancelDialog( void )
{
  this->hide( );
}
