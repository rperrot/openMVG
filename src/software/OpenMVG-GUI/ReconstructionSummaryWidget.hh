#ifndef _OPENMVG_SOFTWARE_OPENMVG_GUI_RECONSTRUCTION_SUMMARY_WIDGET_HH_
#define _OPENMVG_SOFTWARE_OPENMVG_GUI_RECONSTRUCTION_SUMMARY_WIDGET_HH_

#include <QWebEngineView>
#include <QWidget>

namespace openMVG_gui
{

/**
 * @brief Class used to show results of the reconstruction process
 */
class ReconstructionSummaryWidget : public QWidget
{
  public:

    /**
     * @brief Ctr
     * @param path Reconstruction summary path
     * @param parent parent widget
     */
    ReconstructionSummaryWidget( QWidget * parent = nullptr ) ;

    /**
     * @brief Set path where the summary is
     * @param path Path
     */
    void setPath( const std::string & path ) ;

  private:

    /**
     * @brief build interface widgets
     */
    void buildInterface( void ) ;

    QWebEngineView * m_view ;
} ;

} // namespace openMVG_gui

#endif