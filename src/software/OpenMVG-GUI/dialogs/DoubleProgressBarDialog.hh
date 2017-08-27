#ifndef _OPENMVG_SOFTWARE_OPENMVG_GUI_DIALOGS_DOUBLE_PROGRESS_BAR_DIALOG_HH_
#define _OPENMVG_SOFTWARE_OPENMVG_GUI_DIALOGS_DOUBLE_PROGRESS_BAR_DIALOG_HH_

#include <QDialog>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>

namespace openMVG_gui
{

/**
* @brief a progress dialog with two progress bar
*/
class DoubleProgressBarDialog : public QDialog
{
  public:

    /**
    * @brief Ctr
    * @param parent Parent widget
    */
    DoubleProgressBarDialog( QWidget * parent ) ;

    /**
    * @brief Indicate if progress was canceled (either by cancel button or by cancel slot)
    */
    bool wasCanceled( void ) ;

  public slots:

    /**
    * @brief Set progress bar range for first progress bar
    * @param min Minimum value
    * @param max Maximum value
    */
    void setRange1( int min , int max ) ;

    /**
    * @brief Set progress bar range for second progress bar
    * @param min Minimum value
    * @param max Maximum value
    */
    void setRange2( int min , int max ) ;

    /**
    * @brief Set value of first progress bar
    * @param value Value to set
    */
    void setValue1( int value ) ;

    /**
    * @brief Set value of second progress bar
    * @param value Value to set
    */
    void setValue2( int value ) ;

    /**
    * @brief Set label for first progress bar
    * @param value New label
    */
    void setLabelText1( const std::string & value ) ;

    /**
    * @brief Set label for second progress bar
    * @param value New label
    */
    void setLabelText2( const std::string & value ) ;

    /**
    * @brief Cancel (ie: hide) progress bar
    */
    void cancel( void );

  signals:

    void canceled() ;

  private:

    /**
    * @brief Build interface
    */
    void buildInterface( void ) ;

    /**
    * @brief Make connections between widgets
    */
    void makeConnections( void ) ;

    QLabel * m_label_1 ;
    QProgressBar * m_bar_1 ;

    QLabel * m_label_2 ;
    QProgressBar * m_bar_2 ;

    QPushButton * m_push_1 ;

    bool m_canceled ;

    Q_OBJECT
} ;

} // namespace openMVG_gui

#endif