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

#include "DataManager.h"
#include "DomainManager.h"
#include "InteractionManager.h"
#include "layouts/LayoutManager.h"
#include "Loggers.h"
#include "PaneManager.h"
#include "RepresentationCreatorManager.h"
#include "reps/Item.h"
#include "reps/ConnectivityRep.h"
#include "SelectionManager.h"
#include "ZeroEQManager.h"
//#include "domains/domains.h"
#include <unordered_set>

#include <QGuiApplication>

namespace nslib
{
  QMenu* InteractionManager::_contextMenu = nullptr;
  ConnectionRelationshipEditWidget*
    InteractionManager::_conRelationshipEditWidget = nullptr;
  ChildrenRelationshipEditWidget*
    InteractionManager::_editChildrenRelationship = nullptr;
  EntityEditWidget* InteractionManager::_entityEditWidget = nullptr;
  QGraphicsItem* InteractionManager::_item = nullptr;
  Qt::MouseButtons InteractionManager::_buttons = 0;
  std::unique_ptr< TemporalConnectionLine > InteractionManager::_tmpConnectionLine =
    std::unique_ptr< TemporalConnectionLine >( new TemporalConnectionLine( ));
  QAbstractGraphicsShapeItem* InteractionManager::lastShapeItemHoveredOnMouseMove =
    nullptr;

  void InteractionManager::highlightConnectivity(
    QAbstractGraphicsShapeItem* shapeItem, bool highlight )
  {
    auto& relConnectsTo = *( DataManager::entities( ).
                             relationships( )[ "connectsTo" ]->asOneToN( ));
    auto& relConnectedBy = *( DataManager::entities( ).
                              relationships( )[ "connectedBy" ]->asOneToN( ));

    // Third parameter indicates if the relationship has to be inverted
    enum { HLC_RELATIONSHIP = 0, HLC_COLOR = 1, HLC_INVERT = 2 };
    std::vector< std::tuple< shift::RelationshipOneToN*, scoop::Color, bool >> rels;
    rels.reserve( 2 );
    rels.push_back(
      std::make_tuple( &relConnectsTo, scoop::Color( 0, 204, 255 ), false ));
    rels.push_back(
      std::make_tuple( &relConnectedBy, scoop::Color( 255, 204, 0 ), true ));

    const auto& repsToEntities =
      RepresentationCreatorManager::repsToEntities( );
    const auto& relatedEntities =
      RepresentationCreatorManager::relatedEntities( );

    auto item = dynamic_cast< Item* >( shapeItem );
    if ( item )
    {
      assert( item->parentRep( ));

      const auto& entities = repsToEntities.find( item->parentRep( ));
      if ( entities != repsToEntities.end( ))
      {
        auto entityGid = ( *entities->second.begin( ))->entityGid( );
        for ( const auto& relPair : rels )
        {
          auto& rel = *( std::get< HLC_RELATIONSHIP >( relPair ));
          auto& connectingEntities = rel[ entityGid ];
          for ( auto& connectingEntity : connectingEntities )
          {
            const auto& relationRep = relatedEntities.find(
              ( std::get< HLC_INVERT >( relPair ) ?
                std::make_pair( connectingEntity.first, entityGid ) :
                std::make_pair( entityGid, connectingEntity.first )));
            if ( relationRep != relatedEntities.end( ))
            {
              shift::Representation* rep = std::get< 0 >( relationRep->second );
              auto* connRep = dynamic_cast< ConnectivityRep* >( rep );
              if ( connRep )
              {
                if ( highlight )
                  connRep->highlight( std::get< HLC_COLOR >( relPair ));
                else
                  connRep->unHighlight( );
              }
            }
          } // for all connectinf entities
        }
      }
    }
  }





  void InteractionManager::hoverEnterEvent(
    QAbstractGraphicsShapeItem* shapeItem,
    QGraphicsSceneHoverEvent* event )
  {
    // std::cout << "hover" << std::endl;
    auto selectableItem = dynamic_cast< SelectableItem* >( shapeItem );
    if ( selectableItem )
    {
      selectableItem->hover( true );
      selectableItem->setSelected( selectableItem->selectedState( ));
    }
    else
    {
      shapeItem->setPen( SelectableItem::hoverUnselectedPen( ));
    }

    if ( event && event->modifiers( ).testFlag( Qt::ControlModifier ))
    {
      if ( _entityEditWidget )
        delete _entityEditWidget;
      auto item = dynamic_cast< Item* >( shapeItem );
      if ( item )
      {
        assert( item->parentRep( ));
        const auto& repsToEntities =
          RepresentationCreatorManager::repsToEntities( );
        if ( repsToEntities.find( item->parentRep( )) != repsToEntities.end( ))
        {
          const auto entities = repsToEntities.at( item->parentRep( ));
          auto entityGid = ( *entities.begin( ))->entityGid( );
          _entityEditWidget = new EntityEditWidget(
            DataManager::entities( ).at( entityGid ),
            EntityEditWidget::TEntityEditWidgetAction::EDIT_ENTITY );
          EntityEditWidget::parentDock( )->setWidget( _entityEditWidget );
          EntityEditWidget::parentDock( )->show( );
          _entityEditWidget->show( );
        }
      }
    }
  }


  void InteractionManager::hoverLeaveEvent(
    QAbstractGraphicsShapeItem* item,
    QGraphicsSceneHoverEvent* /* event */ )
  {
    if ( !item )
      return;

    auto selectableItem = dynamic_cast< SelectableItem* >( item );
    if ( selectableItem )
    {
      selectableItem->hover( false );
      selectableItem->setSelected( selectableItem->selectedState( ));
    }
    else
    {
      item->setPen( SelectableItem::unselectedPen( ));
    }
  }


  void InteractionManager::contextMenuEvent(
    QAbstractGraphicsShapeItem* shapeItem,
    QGraphicsSceneContextMenuEvent* event )
  {

    if ( !_contextMenu )
      _contextMenu = new QMenu( );
    else
      _contextMenu->clear( );

    // If clicking outside item, new item menu is showed
    if ( !shapeItem )
    {
      auto domain = DomainManager::getActiveDomain( );
      if ( domain )
      {
        const auto& entitiesTypes = domain->entitiesTypes( ).entitiesTypes( );

        std::unordered_map< QAction*, unsigned int > actionToIdx;
        unsigned int entityIdx = 0;

        for ( const auto& type : entitiesTypes )
        {
          QAction* action = nullptr;

          if ( !std::get< shift::EntitiesTypes::IS_SUBENTITY >( type ))
          {
            action = _contextMenu->addAction(
              QString( "Add " ) +QString::fromStdString(
              std::get< shift::EntitiesTypes::ENTITY_NAME >( type )));

            actionToIdx[ action ] = entityIdx;
            ++entityIdx;

          }
        }
        QAction* selectedAction =_contextMenu->exec( event->screenPos( ));
        if ( selectedAction )
        {

          if ( _entityEditWidget )
            delete _entityEditWidget;

          auto& relChildOf = *( DataManager::entities( ).relationships( )
            [ "isChildOf" ]->asOneToOne( ));
          const auto& sceneEntities = PaneManager::activePane( )->entities( ).vector();
          int commonParent = -1;
          if ( !sceneEntities.empty( ))
          {
            commonParent = ( int ) relChildOf[ sceneEntities.front( )
              ->entityGid( ) ].entity;

            for ( auto sceneEntity : sceneEntities )
            {
              if ( commonParent !=
                ( int ) relChildOf[ sceneEntity->entityGid( ) ].entity )
              {
                commonParent = -1;
                break;
              }

            }
          }
          shift::Entity* parentEntity = (commonParent <= 0)
            ? nullptr : DataManager::entities( ).at( commonParent );

          _entityEditWidget = new EntityEditWidget(
            std::get< shift::EntitiesTypes::OBJECT >(
              entitiesTypes[actionToIdx[selectedAction]]),
            EntityEditWidget::TEntityEditWidgetAction::NEW_ENTITY,
            nullptr, true, parentEntity );
          EntityEditWidget::parentDock( )->setWidget( _entityEditWidget );
          EntityEditWidget::parentDock( )->show( );
          _entityEditWidget->show( );
        }
      } // if ( domain )
    }
    else
    {
      auto item = dynamic_cast< Item* >( shapeItem );
      if ( item )
      {
        assert( item->parentRep( ));
        const auto& repsToEntities =
          RepresentationCreatorManager::repsToEntities( );
        if ( repsToEntities.find( item->parentRep( )) != repsToEntities.end( ))
        {
          const auto entities = repsToEntities.at( item->parentRep( ));
          auto entityGid = ( *entities.begin( ))->entityGid( );

          auto& relParentOf = *( DataManager::entities( ).
                                 relationships( )[ "isParentOf" ]->asOneToN( ));
          const auto& children = relParentOf[ entityGid ];

          auto& relChildOf = *( DataManager::entities( ).relationships( )
                                [ "isChildOf" ]->asOneToOne( ));
          const auto& parent = relChildOf[ entityGid ].entity;

          const auto& grandParent =
            relChildOf[ relChildOf[ entityGid ].entity ].entity;

          const auto& parentSiblings = relParentOf[ grandParent ];

          auto& relAGroupOf = *( DataManager::entities( ).relationships( )
                                [ "isAGroupOf" ]->asOneToN( ));
          const auto& groupedEntities = relAGroupOf[ entityGid ];

          QAction* editEntity = nullptr;
          QAction* dupEntity = nullptr;
          QAction* autoEntity = nullptr;
          //QAction* childrenEntity = nullptr; TODO add drag and drop-esque functionality
          auto entity = DataManager::entities( ).at( entityGid );
          if ( !entity->isSubEntity( ))
          {
            editEntity = _contextMenu->addAction( "Edit" );
            dupEntity = _contextMenu->addAction( "Duplicate" );
              //TODO connection restricition support
              if(entity->hasProperty( "Neuron model" ))
              {
                autoEntity = _contextMenu->addAction( "Add Auto Connection" );
              }

              /** TODO add drag and drop-esque functionality
              else if( entity->hasProperty( "Nb of neurons Mean" ) )
              {
                childrenEntities =
                  _contextMenu->addAction( "Add selection as children" );
              }*/

            }

         /* shift::RelationshipPropertiesTypes::rel_constr_range n;
          n = shift::RelationshipPropertiesTypes::constraints( "ParentOf","NeuronSuperPop");
          for (auto it=n.first; it!=n.second; ++it){
            std::cout << "child: ";
            std::cout << it->second << std::endl;
          }*/
          if ( editEntity || dupEntity || autoEntity
            /** || childrenEntity //TODO add drag and drop-esque functionality*/)
            _contextMenu->addSeparator( );

          QAction* childAction = nullptr;

          unsigned int entityIdx = 0;
          std::unordered_map< QAction*, unsigned int > actionToIdx;

		  /*
          if( entity->hasProperty( "Nb of neurons Mean" ) )
          {
            auto domain = DomainManager::getActiveDomain( );

            if ( domain )
            {
              entitiesTypes = domain->entitiesTypes( ).entitiesTypes( );

              for ( const auto& type : entitiesTypes )
              {
                if ( !std::get< shift::EntitiesTypes::IS_SUBENTITY >( type ) )
                {
                  childAction = _contextMenu->addAction(
                    QString( "Add " ) +  QString::fromStdString(
                    std::get< shift::EntitiesTypes::ENTITY_NAME >( type ) ) +
                    QString( " as child" ) );

                  actionToIdx[ childAction ] = entityIdx;
                  ++entityIdx;
                }
              }
            }
            _contextMenu->addSeparator( );
          }*/
          auto domain = DomainManager::getActiveDomain( );
          auto entitiesTypes = new std::vector<shift::Entity*>;
          if ( domain ) {
            auto domainEntitiesTypes = domain->entitiesTypes();
            std::cout << "Type detected: " << domainEntitiesTypes.entityTypeName(entity) << std::endl;
            shift::RelationshipPropertiesTypes::rel_constr_range childrenTypesNames;
            childrenTypesNames = shift::RelationshipPropertiesTypes::constraints("ParentOf",
              domainEntitiesTypes.entityTypeName( entity ));
            for (auto childTypeName = childrenTypesNames.first;
                 childTypeName != childrenTypesNames.second; ++childTypeName) {
              std::cout << "child: " << childTypeName->second << std::endl;
              childAction = _contextMenu->addAction(
                      QString("Add ") + QString::fromStdString(
                              childTypeName->second) + QString(" as child"));
              actionToIdx[childAction] = entityIdx;
                entitiesTypes->push_back(domainEntitiesTypes.getEntityObject(childTypeName->second));
              ++entityIdx;
            }
            if (childAction) {
              _contextMenu->addSeparator();
            }
          }

          QAction* levelUp = ( parent != 0 )
            ? _contextMenu->addAction( "Level up" ) : nullptr;
          QAction* levelDown = ( !children.empty( ))
            ? _contextMenu->addAction( "Level down" ) : nullptr;
          QAction* expandGroup = ( !groupedEntities.empty( ))
            ? _contextMenu->addAction( "Expand group" ) : nullptr;

          if ( levelUp || levelDown || expandGroup )
          {
            _contextMenu->addSeparator( );
          }

          QAction* levelUpToNewPane = ( parent != 0 )
            ? _contextMenu->addAction( "Level up [new pane]" ) : nullptr;
          QAction* levelDownToNewPane = ( !children.empty( ))
            ? _contextMenu->addAction( "Level down [new pane]" ) : nullptr;
          QAction* expandGroupToNewPane = ( !groupedEntities.empty( ))
            ? _contextMenu->addAction( "Expand group [new pane]" ) : nullptr;

          if ( levelUp || levelDown || expandGroup || levelUpToNewPane ||
              levelDownToNewPane || expandGroupToNewPane || editEntity )
          {
            shift::Representations representations;
            shift::Entities targetEntities;
            QAction* selectedAction = _contextMenu->exec( event->screenPos( ));

            if ( editEntity && editEntity == selectedAction )
            {
              if ( _entityEditWidget )
                delete _entityEditWidget;

              _entityEditWidget = new EntityEditWidget(
                DataManager::entities( ).at( entityGid ),
                EntityEditWidget::TEntityEditWidgetAction::EDIT_ENTITY );
              EntityEditWidget::parentDock( )->setWidget( _entityEditWidget );
              EntityEditWidget::parentDock( )->show( );
              _entityEditWidget->show( );
            }
            else if ( dupEntity && dupEntity == selectedAction )
            {
              if ( _entityEditWidget )
                delete _entityEditWidget;

              _entityEditWidget = new EntityEditWidget(
                DataManager::entities( ).at( entityGid ),
                EntityEditWidget::TEntityEditWidgetAction::DUPLICATE_ENTITY );
              EntityEditWidget::parentDock( )->setWidget( _entityEditWidget );
              EntityEditWidget::parentDock( )->show( );
              _entityEditWidget->show( );

            }
            else if ( autoEntity && autoEntity == selectedAction )
            {
              createConnectionRelationship(
                DataManager::entities( ).at( entityGid ),
                DataManager::entities( ).at( entityGid )
              );
            }
            /** TODO add drag and drop-esque functionality
            else if ( childrenEntity && childrenEntity == selectedAction )
            {
              editChildrenRelationship(
                entity,SelectionManager::getActiveSelection());
            }*/
            else if ( ( levelUp && levelUp == selectedAction ) ||
              ( levelUpToNewPane && levelUpToNewPane == selectedAction ))
            {
              if( levelUpToNewPane && levelUpToNewPane == selectedAction )
              {
                nslib::PaneManager::activePane(
                  nslib::PaneManager::newPaneFromActivePane( ));
              }
              if ( parentSiblings.size( ) > 0 )
              {
                for( const auto& parentSibling : parentSiblings )
                  targetEntities.add(
                    DataManager::entities( ).at( parentSibling.first ));
              }
              else if ( grandParent == 0 )
              {
                for( const auto& rootEntity
                  : DataManager::rootEntities( ).vector( ))
                {
                  targetEntities.add( rootEntity );
                }
              }
              else
              {
                targetEntities.add( DataManager::entities( ).at( parent ));
              }
            }
            else if ( ( levelDown && levelDown == selectedAction ) ||
              ( levelDownToNewPane && levelDownToNewPane == selectedAction ))
            {
              if ( levelDownToNewPane &&
                  levelDownToNewPane == selectedAction )
              {
                nslib::PaneManager::activePane(
                    nslib::PaneManager::newPaneFromActivePane( ) );
              }

              for ( const auto& child : children )
                targetEntities.add( DataManager::entities( ).at( child.first ));
            }
            else if ( ( expandGroup && expandGroup == selectedAction ) ||
              ( expandGroupToNewPane &&
              expandGroupToNewPane == selectedAction ))
            {
              if ( expandGroupToNewPane &&
                expandGroupToNewPane == selectedAction )
              {
                nslib::PaneManager::activePane(
                  nslib::PaneManager::newPaneFromActivePane( ));
              }
              for ( const auto& groupedEntity : groupedEntities )
                targetEntities.add(
                  DataManager::entities( ).at( groupedEntity.first ));
            }
            else if ( selectedAction )
            {
              if ( _entityEditWidget )
              {
                delete _entityEditWidget;
              }
              _entityEditWidget = new EntityEditWidget(
                entitiesTypes->at( actionToIdx[ selectedAction ] ),
                EntityEditWidget::TEntityEditWidgetAction::NEW_ENTITY, nullptr,
                false, entity );

              EntityEditWidget::parentDock( )->setWidget( _entityEditWidget );
              EntityEditWidget::parentDock( )->show( );
              _entityEditWidget->show( );
            }
            if ( targetEntities.size( ) > 0 )
            {
              auto canvas = PaneManager::activePane( );
              canvas->displayEntities( targetEntities, false, true );
            }
          }
        }
        else
        {
          Loggers::get( )->log( "item without entity",
            LOG_LEVEL_ERROR, NEUROSCHEME_FILE_LINE );
          return;
        }
      }
      else
      {
        Loggers::get( )->log( "Clicked element is not item",
          LOG_LEVEL_ERROR, NEUROSCHEME_FILE_LINE );
      }
    }

    if ( _tmpConnectionLine && _tmpConnectionLine->scene( ))
    {
      _tmpConnectionLine->scene( )->removeItem( _tmpConnectionLine.get( ));
    }

  } // context menu


  void InteractionManager::mousePressEvent( QGraphicsItem* item,
                                            QMouseEvent* event )
  {
    if ( item )
    {
      // _tmpConnectionLine.reset(
      //   new QGraphicsLineItem( QLineF( item->scenePos( ), item->scenePos( ))));

      if ( _tmpConnectionLine )
      {
        _tmpConnectionLine->setLine( QLineF( item->scenePos( ), item->scenePos( )));
        _tmpConnectionLine->setZValue( -100000 );
        _tmpConnectionLine->setPen( QPen( QColor( 128, 128, 128),
          nslib::Config::scale( ), Qt::DotLine ));

        auto scene = item->scene( );
        scene->addItem( _tmpConnectionLine.get( ));
      }

      auto parentItem = item->parentItem( );
      while ( parentItem )
      {
        auto selectableItem = dynamic_cast< SelectableItem*>( item );
        if ( selectableItem )
          break;
        item = parentItem;
        parentItem = item->parentItem( );
      }
      _item = item;
      _buttons = event->buttons( );
    }
    else
    {
      _item = nullptr;
      _buttons = 0;
    }
  }

  void InteractionManager::mouseMoveEvent( QGraphicsView* graphicsView,
                                           QAbstractGraphicsShapeItem* shapeItem,
                                           QMouseEvent* event )
  {
    // It _item has value means that a link its being drawn
    if ( _item && _tmpConnectionLine )
    {
      const auto& initPoint = _tmpConnectionLine->line( ).p1( );
      auto newPos = graphicsView->mapToScene( event->pos( ));
      _tmpConnectionLine->setLine( QLineF( initPoint, newPos ));

      if ( shapeItem && dynamic_cast< Item* >( shapeItem ))
      {
        InteractionManager::hoverLeaveEvent( lastShapeItemHoveredOnMouseMove,
                                             nullptr );
        InteractionManager::hoverEnterEvent( shapeItem, nullptr );
        lastShapeItemHoveredOnMouseMove = shapeItem;
      }
    }
  }

  void InteractionManager::mouseReleaseEvent( QGraphicsItem* item_,
                                              QMouseEvent* /*event*/ )
  {

    if( item_ && _item )
    {
      auto parentItem = item_->parentItem( );
      while ( parentItem )
      {
        auto selectableItem = dynamic_cast< SelectableItem*>( item_ );
        if (selectableItem)
          break;
        item_ = parentItem;
        parentItem = item_->parentItem( );
      }

      if ( _item == item_ )
      {
        // Selection event
        if ( _buttons & Qt::LeftButton )
        {
          auto item = dynamic_cast< Item* >( item_ );

          auto selectableItem = dynamic_cast< SelectableItem* >( item );
          if ( selectableItem )
          {
            const auto& repsToEntities =
              RepresentationCreatorManager::repsToEntities( );
            if ( repsToEntities.find( item->parentRep( )) !=
                 repsToEntities.end( ))
            {
              const auto entities = repsToEntities.at( item->parentRep( ));
              if ( selectableItem->partiallySelected( ))
                selectableItem->setSelected( );
              else
                selectableItem->toggleSelected( );

// std::cout << "-------- Selecting entities "
//           << entities.size( ) << std::endl;
              for ( const auto& entity : entities )
              {
// std::cout << "-------- " << entity->entityGid( ) << std::endl;
// std::cout << "-- ShiFT gid: "
//           << int( entity->entityGid( )) << std::endl;

                if ( selectableItem->selected( ))
                {
                  SelectionManager::setSelectedState(
                    entity, SelectedState::SELECTED );
                }
                else
                {
                  SelectionManager::setSelectedState(
                    entity, SelectedState::UNSELECTED );
                }

                auto entityState = SelectionManager::getSelectedState( entity );
                // auto entityGid = ( *entities.begin( ))->entityGid( );
                auto entityGid = entity->entityGid( );

                const auto& allEntities = DataManager::entities( );
                const auto& relationships = allEntities.relationships( );
                const auto& relChildOf =
                  *( relationships.at( "isChildOf" )->asOneToOne( ));
                const auto& relParentOf =
                  *( relationships.at( "isParentOf" )->asOneToN( ));
                const auto& relSubEntityOf =
                  *( relationships.at( "isSubEntityOf" )->asOneToOne( ));
                const auto& relSuperEntityOf =
                  *( relationships.at( "isSuperEntityOf" )->asOneToN( ));
                const auto& relAGroupOf =
                  *( relationships.at( "isAGroupOf" )->asOneToN( ));

                if ( relSubEntityOf.count( entityGid ) > 0 )
                {
                  std::unordered_set< unsigned int > parentIds;
                  if ( relAGroupOf.count( entityGid ) > 0 )
                  {
                    const auto& groupedIds = relAGroupOf.at( entityGid );
                    // std::cout << " -- Parent of: ";
                    // std::cout << ",,,, Grouped " << groupedIds.size( ) << std::endl;
                    for ( auto const& groupedId : groupedIds )
                    {
                      SelectionManager::setSelectedState(
                        allEntities.at( groupedId.first ), entityState );
                      // Save unique parent set for updating only once per parent
                      if ( relChildOf.count( groupedId.first ) > 0 )
                        parentIds.insert( relChildOf.at(
                                            groupedId.first ).entity );
                    }
                    _updateSelectedStateOfSubEntities(
                      allEntities, relSuperEntityOf, relAGroupOf,
                      relSubEntityOf.at( entityGid ).entity );
                    std::unordered_set< unsigned int > uniqueParentChildIds;
                    for ( auto const& parentId : parentIds )
                    {
                      uniqueParentChildIds.insert(
                        relParentOf.at( parentId ).begin( )->first );
                    }
                    // std::cout << ",,,, Parents: " << parentIds.size( ) << std::endl;
                    for ( auto const& uniqueParentChildId : uniqueParentChildIds )
                    {
                      _propagateSelectedStateToParent(
                        allEntities, relChildOf, relParentOf,
                        relSuperEntityOf, relAGroupOf,
                        uniqueParentChildId, entityState );
                    }
                  }
                } // if subentity
                else
                {
                  if ( relSuperEntityOf.count( entityGid ) > 0 )
                  {
                    const auto& subEntities =
                      relSuperEntityOf.at( entityGid );
                    for ( const auto& subEntity : subEntities )
                      SelectionManager::setSelectedState(
                        allEntities.at( subEntity.first ), entityState );
                  }

                  // std::cout << "Propagate to children of " << entityGid << std::endl;
                  _propagateSelectedStateToChilds(
                    allEntities, relParentOf, relSuperEntityOf,
                    entityGid, entityState );

                  _propagateSelectedStateToParent(
                    allEntities, relChildOf, relParentOf,
                    relSuperEntityOf, relAGroupOf,
                    entityGid, entityState );
                  //std::cout << std::endl;
                }
                //LayoutManager::updateAllScenesSelection( );
                PaneManager::updateSelection( );
              }
            }
            else
            {
              Loggers::get( )->log( "item without entity",
                                    LOG_LEVEL_ERROR, NEUROSCHEME_FILE_LINE );
              return;
            }
          }

          // Publish selection
          std::vector< unsigned int > ids;
          SelectionManager::selectableEntitiesIds( ids );

          if ( Config::autoPublishSelection( ))
            ZeroEQManager::publishSelection( ids );
          if ( Config::autoPublishFocusOnSelection( ))
            ZeroEQManager::publishFocusOnSelection( ids );
        } // selection event
      }
      else
      {
        // drag and drop
        if ( _buttons & Qt::LeftButton )
        {
          auto originItem = dynamic_cast< Item* >( _item );
          auto destinationItem = dynamic_cast< Item* >( item_ );

          if ( destinationItem )
          {
            const auto& repsToEntities =
              RepresentationCreatorManager::repsToEntities( );
            if (( repsToEntities.find( originItem->parentRep( )) !=
                  repsToEntities.end( )) &&
                ( repsToEntities.find( destinationItem->parentRep( )) !=
                  repsToEntities.end( )))
            {
              const auto originEntity =
                *( repsToEntities.at( originItem->parentRep( )).begin( ));
              const auto destinationEntity =
                *( repsToEntities.at( destinationItem->parentRep( )).begin( ));

              createConnectionRelationship( originEntity, destinationEntity );
            }
          }
        }
      }
    }

    if ( _tmpConnectionLine && _tmpConnectionLine->scene( ))
      _tmpConnectionLine->scene( )->removeItem( _tmpConnectionLine.get( ));

    _item = nullptr;
    _buttons = 0;


  }

  void InteractionManager::createConnectionRelationship(
    shift::Entity* originEntity_, shift::Entity* destinationEntity_ )
  {
    if ( _conRelationshipEditWidget )
      delete _conRelationshipEditWidget;

    _conRelationshipEditWidget =
      new ConnectionRelationshipEditWidget( originEntity_, destinationEntity_,
        &PaneManager::activePane()->view(), true);
    _conRelationshipEditWidget->show( );
  }

  void InteractionManager::editChildrenRelationship(
      shift::Entity* parentEntity_,  std::vector< shift::Entity* > childrenEntities_ )
  {
    if ( _editChildrenRelationship )
      delete _editChildrenRelationship;
    _editChildrenRelationship =
        new ChildrenRelationshipEditWidget( parentEntity_, childrenEntities_,
          &PaneManager::activePane()->view(), true);
    _editChildrenRelationship->show( );
  }

  void InteractionManager::_propagateSelectedStateToChilds(
    const shift::Entities& entities,
    const shift::RelationshipOneToN& relParentOf,
    const shift::RelationshipOneToN& relSuperEntityOf,
    unsigned int entityGid,
    SelectedState state )
  {
    if ( relParentOf.count( entityGid ) == 0 )
      return;
    const auto& childrenIds = relParentOf.at( entityGid );
    // std::cout << " -- Parent of: ";
    for ( auto const& childId : childrenIds )
    {
      // std::cout << childId << " ";
      if ( relSuperEntityOf.count( childId.first ) > 0 )
      {
        const auto& subEntities = relSuperEntityOf.at( childId.first );
        for ( const auto& subEntity : subEntities )
          SelectionManager::setSelectedState(
            DataManager::entities( ).at( subEntity.first ), state );
      }
      SelectionManager::setSelectedState(
        entities.at( childId.first ), state );
      _propagateSelectedStateToChilds( entities, relParentOf, relSuperEntityOf,
                                       childId.first, state );
    }

  }

  void InteractionManager::_propagateSelectedStateToParent(
    const shift::Entities& entities,
    const shift::RelationshipOneToOne& relChildOf,
    const shift::RelationshipOneToN& relParentOf,
    const shift::RelationshipOneToN& relSuperEntityOf,
    const shift::RelationshipOneToN& relAGroupOf,
    unsigned int entityGid,
    SelectedState childState )
  {
    if ( relChildOf.count( entityGid ) == 0 )
      return;
    const auto& parentId = relChildOf.at( entityGid ).entity;

    if ( parentId == 0 ) return;

    if ( childState == SelectedState::PARTIALLY_SELECTED )
    {
      //std::cout << "<>Partially selected" << std::endl;
      SelectionManager::setSelectedState(
        entities.at( parentId ), childState );
      _propagateSelectedStateToParent( entities, relChildOf, relParentOf,
                                       relSuperEntityOf, relAGroupOf,
                                       parentId, childState );
      return;
    }

    bool allChildrenSelected, noChildrenSelected;
    queryChildrenSelectedState( entities, relParentOf, parentId,
                                allChildrenSelected, noChildrenSelected );
    //std::cout << "<>AllChildSelected? = " << allChildrenSelected << std::endl;
    SelectedState state;
    if ( noChildrenSelected )
      state = SelectedState::UNSELECTED;
    else if ( allChildrenSelected )
      state = SelectedState::SELECTED;
    else
      state = SelectedState::PARTIALLY_SELECTED;

    SelectionManager::setSelectedState(
      entities.at( parentId ), state );

    _updateSelectedStateOfSubEntities(
      entities, relSuperEntityOf, relAGroupOf, parentId );

    _propagateSelectedStateToParent( entities, relChildOf, relParentOf,
                                     relSuperEntityOf, relAGroupOf,
                                     parentId, state );
  }

  void InteractionManager::_updateSelectedStateOfSubEntities(
    const shift::Entities& entities,
    const shift::RelationshipOneToN& relSuperEntityOf,
    const shift::RelationshipOneToN& relAGroupOf,
    unsigned int entityGid )
  {
    if ( relSuperEntityOf.count( entityGid ) < 1 )
      return;

    for ( const auto& subEntityId : relSuperEntityOf.at( entityGid ))
    {
      bool allGroupedSelected, noGroupedSelected;
      _queryGroupedSelectedState( entities, relAGroupOf, subEntityId.first,
                                  allGroupedSelected, noGroupedSelected );
      SelectedState groupedState;
      if ( noGroupedSelected )
        groupedState = SelectedState::UNSELECTED;
      else if ( allGroupedSelected )
        groupedState = SelectedState::SELECTED;
      else
        groupedState = SelectedState::PARTIALLY_SELECTED;

      SelectionManager::setSelectedState(
        entities.at( subEntityId.first ), groupedState );

    }
  }

  void InteractionManager::queryChildrenSelectedState(
    const shift::Entities& entities,
    const shift::RelationshipOneToN& relParentOf,
    unsigned int entityGid,
    bool& allChildrenSelected,
    bool& noChildrenSelected )
  {
    allChildrenSelected = true;
    noChildrenSelected = true;
    const auto& childrenIds = relParentOf.at( entityGid );
    for ( auto const& childId : childrenIds )
    {
      if ( SelectionManager::getSelectedState( entities.at( childId.first )) !=
           SelectedState::SELECTED )
        allChildrenSelected = false;
      if ( SelectionManager::getSelectedState( entities.at( childId.first )) ==
           SelectedState::SELECTED )
        noChildrenSelected = false;
    }
    return;
  }

  void InteractionManager::_queryGroupedSelectedState(
    const shift::Entities& entities,
    const shift::RelationshipOneToN& relAGroupOf,
    unsigned int entityGid,
    bool& allGroupedSelected,
    bool& noGroupedSelected )
  {
    allGroupedSelected = true;
    noGroupedSelected = true;
    if ( relAGroupOf.count( entityGid ) == 0 )
    {
      allGroupedSelected = false;
      noGroupedSelected = true;
      return;
    }
    const auto& groupedIds = relAGroupOf.at( entityGid );
    for ( auto const& groupedId : groupedIds )
    {
      if ( SelectionManager::getSelectedState( entities.at( groupedId.first )) !=
           SelectedState::SELECTED )
        allGroupedSelected = false;
      if ( SelectionManager::getSelectedState( entities.at( groupedId.first )) ==
           SelectedState::SELECTED )
        noGroupedSelected = false;
    }
  }

} // namespace nslib
