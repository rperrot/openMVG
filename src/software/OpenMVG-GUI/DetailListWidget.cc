// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2017 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "DetailListWidget.hh"

#include <QHeaderView>

#include <iomanip>
#include <iostream>
#include <sstream>

namespace openMVG_gui
{

/**
* @brief Ctr
* @param parent Parent widget
*/
DetailListWidget::DetailListWidget( QWidget * parent )
  : QTreeWidget( parent )
{
  BuildInterface() ;
  setMinimumSize( 310 , 10 ) ;
}


/**
* @brief Set Images information
* @param names Name of the images
* @param resolution (width,height) of the images
*/
void DetailListWidget::setImagesInfos( std::map< int , std::string > & names ,
                                       std::map<int, std::pair<int, int> > & resolution )
{
  // Remove existing childs
  m_item_images->takeChildren() ;

  for( auto & it : names )
  {
    const int cur_id = it.first ;
    const std::string & cur_name = it.second ;

    QTreeWidgetItem * item = new QTreeWidgetItem( m_item_images ) ;
    item->setText( 0 , std::to_string( cur_id ).c_str() ) ;

    QTreeWidgetItem * itemName = new QTreeWidgetItem( item ) ;
    itemName->setText( 0 , "Name" ) ;
    itemName->setText( 1 , cur_name.c_str() ) ;
    itemName->setData( 1 , Qt::UserRole , cur_id ) ;

    if( resolution.count( cur_id ) )
    {
      QTreeWidgetItem * itemWidth = new QTreeWidgetItem( item ) ;
      itemWidth->setText( 0 , "Width" ) ;
      itemWidth->setText( 1 , std::to_string( resolution.at( cur_id ).first ).c_str() ) ;
      itemWidth->setData( 1 , Qt::UserRole , cur_id ) ;

      QTreeWidgetItem * itemHeight = new QTreeWidgetItem( item ) ;
      itemHeight->setText( 0 , "Height" ) ;
      itemHeight->setText( 1 , std::to_string( resolution.at( cur_id ).second ).c_str() ) ;
      itemHeight->setData( 1 , Qt::UserRole , cur_id ) ;
    }
  }
}

/**
* @brief Set features statistics
* @param stats Statistics
*/
/**
* @brief Set features statistics
* @param show_names Hierarchy for name information (ex : SIFT / SIFT / ULTRA )
* @param stats Statistics (image name, stats)
*/
void DetailListWidget::setFeaturesInfos( const std::vector<std::string> & show_names ,
    const std::map< std::string , FeaturesStats > & stats )
{
  // Get root of the items
  std::vector< std::string > name = { "Detector" , "Descriptor" , "Preset" } ;

  QTreeWidgetItem * last = m_item_features ;
  for( int hierarchy_idx = 0 ; hierarchy_idx < show_names.size() ; ++hierarchy_idx )
  {
    bool found = false ;
    const std::string cur_name_txt = show_names[ hierarchy_idx ] ;

    for( int id = 0 ; id < last->childCount() ; ++id )
    {
      QTreeWidgetItem * item = last->child( id ) ;
      QString text = item->text( 1 ) ;
      if( cur_name_txt == text.toStdString() )
      {
        found = true ;
        last  = item ;
      }
    }

    // If not found, create it
    if( ! found )
    {
      last = new QTreeWidgetItem( last ) ;
      last->setText( 0 , name[ hierarchy_idx ].c_str( ) );
      last->setText( 1 , cur_name_txt.c_str() ) ;
    }
  }

  QTreeWidgetItem * root = last ;
  // Remove alredy computed elements
  root->takeChildren() ;

  for( const auto & cur_it : stats )
  {
    const std::string & cur_name = cur_it.first ;
    const FeaturesStats & cur_stat = cur_it.second ;

    QTreeWidgetItem * item_name = new QTreeWidgetItem( root ) ;
    item_name->setText( 0 , cur_name.c_str() ) ;

    QTreeWidgetItem * item_nb_feat = new QTreeWidgetItem( item_name ) ;
    item_nb_feat->setText( 0 , "Nb features" ) ;
    item_nb_feat->setText( 1 , std::to_string( cur_stat.nbFeature() ).c_str() ) ;
    item_nb_feat->setData( 1 , Qt::UserRole , cur_stat.nbFeature() ) ;

    std::stringstream str ;
    str << std::setprecision( 2 ) << cur_stat.elapsedTime() << " s" ;

    QTreeWidgetItem * item_feat_time = new QTreeWidgetItem( item_name ) ;
    item_feat_time->setText( 0 , "Elapsed" ) ;
    item_feat_time->setText( 1 , str.str().c_str() ) ;
    item_feat_time->setData( 1 , Qt::UserRole , cur_stat.elapsedTime() ) ;
  }
}

/**
* @brief Set Matching computation statistics
* @param stats
*/
void DetailListWidget::setMatchesInfos( const std::vector<std::string> & show_names , const MatchingStats & stats )
{
  // Get root of the items
  std::vector< std::string > name = { "Detector" , "Descriptor" , "Preset" } ;


  QTreeWidgetItem * last = m_item_matches ;
  for( int hierarchy_idx = 0 ; hierarchy_idx < show_names.size() ; ++hierarchy_idx )
  {
    bool found = false ;
    const std::string cur_name_txt = show_names[ hierarchy_idx ] ;

    for( int id = 0 ; id < last->childCount() ; ++id )
    {
      QTreeWidgetItem * item = last->child( id ) ;
      QString text = item->text( 1 ) ;
      if( cur_name_txt == text.toStdString() )
      {
        found = true ;
        last  = item ;
      }
    }

    // If not found, create it
    if( ! found )
    {
      last = new QTreeWidgetItem( last ) ;
      last->setText( 0 , name[ hierarchy_idx ].c_str( ) );
      last->setText( 1 , cur_name_txt.c_str() ) ;
    }
  }

  QTreeWidgetItem * root = last ;
  // Remove already computed elements
  root->takeChildren() ;

  std::stringstream str ;
  str << std::setprecision( 2 ) << stats.putativeElapsedTime() << " s" ;

  std::stringstream str2 ;
  str2 << std::setprecision( 2 ) << stats.filteringElapsedTime() << " s" ;

  QTreeWidgetItem * item_matching = new QTreeWidgetItem( root ) ;
  item_matching->setText( 0 , "Matching" ) ;
  item_matching->setText( 1 , str.str().c_str() ) ;
  item_matching->setData( 1 , Qt::UserRole , stats.putativeElapsedTime() ) ;

  QTreeWidgetItem * item_filtering = new QTreeWidgetItem( root ) ;
  item_filtering->setText( 0 , "Filtering" );
  item_filtering->setText( 1 , str2.str().c_str() ) ;
  item_filtering->setData( 1 , Qt::UserRole , stats.filteringElapsedTime() ) ;
}




// Clear all item except structural items
void DetailListWidget::clear( )
{
  m_item_images->takeChildren() ;
  m_item_features->takeChildren() ;
  m_item_matches->takeChildren() ;
  m_item_reconstruction->takeChildren() ;
}

void DetailListWidget::drawRow( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
  QTreeView::drawRow( painter , option , index ) ;
}


void DetailListWidget::BuildInterface( void )
{
  std::string style =
    "QTreeView {\
    background-color: rgb(64,64,64);\
    alternate-background-color: rgb(100,100,100);\
    color: white;\
    }\
    QTreeView::branch:has-children:!has-siblings:closed, \
    QTreeView::branch:closed:has-children:has-siblings { \
    border-image: none; \
    image: url(:/icons/plus.svg) \
    } \
    \
    QTreeView::branch:open:has-children:!has-siblings, \
    QTreeView::branch:open:has-children:has-siblings  { \
    border-image: none; \
    image: url(:/icons/minus.svg);\
  }";
  setStyleSheet( style.c_str() ) ;

  setAlternatingRowColors( true ) ;
  setColumnCount( 2 );
  header()->close() ;
  header()->setSectionResizeMode( 0, QHeaderView::ResizeToContents );
  header()->setSectionResizeMode( 1, QHeaderView::ResizeToContents );


  m_item_images = new QTreeWidgetItem( this ) ;
  m_item_images->setFirstColumnSpanned( true ) ;
  m_item_images->setText( 0 , "Images" ) ;

  m_item_features = new QTreeWidgetItem( this ) ;
  m_item_features->setFirstColumnSpanned( true ) ;
  m_item_features->setText( 0 , "Features" ) ;

  m_item_matches = new QTreeWidgetItem( this ) ;
  m_item_matches->setFirstColumnSpanned( true ) ;
  m_item_matches->setText( 0 , "Matches" ) ;

  m_item_reconstruction = new QTreeWidgetItem( this ) ;
  m_item_reconstruction->setFirstColumnSpanned( true ) ;
  m_item_reconstruction->setText( 0 , "Reconstruction" ) ;
}


} // namespace openMVG_gui
