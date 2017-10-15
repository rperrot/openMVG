#include "ReconstructionSummaryWidget.hh"

#include <QGridLayout>
#include <QUrl>

#include <iostream>

namespace openMVG_gui
{

/**
 * @brief Ctr
 * @param path Reconstruction summary path
 * @param parent parent widget
 */
ReconstructionSummaryWidget::ReconstructionSummaryWidget( QWidget * parent )
  : QWidget( parent )
{
  buildInterface() ;
  resize( 1024, 600 ) ;
  setWindowTitle( "Reconstuction Summary" ) ;
}

/**
 * @brief Set path where the summary is
 * @param path Path
 */
void ReconstructionSummaryWidget::setPath( const std::string & path )
{
  m_view->load( QUrl::fromLocalFile( path.c_str() ) ) ;
  m_view->show() ;
}

/**
 * @brief build interface widgets
 */
void ReconstructionSummaryWidget::buildInterface( void )
{
  m_view = new QWebEngineView( this ) ;
  m_view->show() ;
  connect( m_view , SIGNAL( loadFinished( bool ) ) , this , SLOT( hasLoadedURL( bool ) ) ) ;

  QGridLayout * layout = new QGridLayout ;

  layout->addWidget( m_view ) ;

  setLayout( layout ) ;
}

void ReconstructionSummaryWidget::hasLoadedURL( bool ok )
{
  if( ! ok )
  {
    std::cerr << "Reconstruction summary : load failure" << std::endl ;
  }
}

} // namespace openMVG_gui
