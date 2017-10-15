#include "ReconstructionSummaryWidget.hh"

#include <QUrl>

#include <QGridLayout>

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
}

/**
 * @brief Set path where the summary is
 * @param path Path
 */
void ReconstructionSummaryWidget::setPath( const std::string & path )
{
  m_view->load( QUrl::fromLocalFile( path.c_str() ) ) ;
}

/**
 * @brief build interface widgets
 */
void ReconstructionSummaryWidget::buildInterface( void )
{
  m_view = new QWebEngineView( this ) ;

  QGridLayout * layout = new QGridLayout ;

  layout->addWidget( m_view ) ;

  setLayout( layout ) ;
}

} // namespace openMVG_gui
