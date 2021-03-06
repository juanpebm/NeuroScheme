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
#ifndef __NEUROSCHEME__MAINWINDOW__
#define __NEUROSCHEME__MAINWINDOW__

#include <ui_MainWindow.h>
#include <QMainWindow>
#include <QMessageBox>
#include <QTableWidget>
#include <QDockWidget>
#include <unordered_map>
#include <nslib/Canvas.h>

namespace Ui
{
    class MainWindow;
}

class StoredSelections
{
public:
  // Stored selections
  QDockWidget* dock;
  QTableWidget* table;
  unsigned int counter;
  std::unordered_map< std::string, QTableWidgetItem* >
  tableWidgets;
};

class MainWindow : public QMainWindow
{
  Q_OBJECT;

public:

  enum TTableColumns
  {
    COLUMN_LABEL = 0,
    COLUMN_COUNT,
    COLUMN_DATETIME,
    COLUMN_MAX_COLUMS
  };

  explicit MainWindow( QWidget *parent , bool zeroEQ );
  ~MainWindow( void );
  void selectDomain( void );

public slots:

  // Stored selections solts
  void aboutDialog( void );
  void updateStoredSelectionsDock( void );
  void sortStoredSelectionsTable( int column );
  void storeSelection( void );
  void deleteStoredSelection( void );
  void restoreSelection( void );
  void updateLayoutsDock( void );
  void killActivePane( void );
  void duplicateActivePane( void );
  void home( void );
  void toggleShowConnectivity( void );
  void toggleShowNoHierarchyEntities( void );
  void toggleShowEntitiesName( void );
  // void saveScene( void );
  void actionPublishSelectionToggle( void );
  void actionPublishFocusOnSelectionToggle( void );
  void actionPublishFocusOnDisplayedToggle( void );
  void actionPublishSelection( void );
  void actionPublishFocusOnSelection( void );
  void actionPublishFocusOnDisplayed( void );
  void toggleZeroEQ( void );
  void quit( void );
  void exportToJSON( void );
  void importFromJSON( void );
  void cleanScene( void );

protected:

  QString _tableColumnToString( TTableColumns column );
  StoredSelections _storedSelections;
  QDockWidget* _layoutsDock = nullptr;
  QDockWidget* _entityEditDock = nullptr;
  QDockWidget* _connectionEditDock = nullptr;
  QDockWidget* _connectionListDock = nullptr;
  QString _lastOpenedFileName;

private:
  const unsigned int minDockSizeX;
  const unsigned int minDockSizeY;
  Ui::MainWindow* _ui;

}; // class MainWindow

#endif // __NEUROSCHEME__MAINWINDOW__
