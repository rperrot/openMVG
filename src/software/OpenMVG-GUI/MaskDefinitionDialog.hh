#ifndef _OPENMVG_SOFTWARE_OPENMVG_GUI_MASK_DEFINITION_DIALOG_HH_
#define _OPENMVG_SOFTWARE_OPENMVG_GUI_MASK_DEFINITION_DIALOG_HH_

#include "MaskView.hh"

#include <QAction>
#include <QCheckBox>
#include <QDialog>
#include <QGraphicsScene>
#include <QPushButton>
#include <QSlider>
#include <QToolBar>

#include <memory>


namespace openMVG_gui
{
class Project ;

class MaskDefinitionDialog : public QDialog
{
  public:

    /**
    * @param parent Parent widget
    * @param project Current project
    * @param id_image Id of the image on which mask will be drawn
    */
    MaskDefinitionDialog( QWidget * parent ,
                          std::shared_ptr<Project> project ,
                          const int id_image ) ;

    /**
    * @brief Indicate if mask is activated
    */
    bool hasMaskActivated( void ) const ;

    /**
    * @brief Activate/Deactivate mask 
    * @param active Activation status
    */ 
    void setMaskActivated( bool active ) ;


    void showEvent( QShowEvent * ) override ;

    /**
    * @brief Get image mask
    */
    QImage getMask( void ) ;

  public slots:
    /**
    * @brief action to be executed when user click on cancel button
    */
    void onCancel( void ) ;

    /**
    * @brief action to be executed when user click on ok button
    */
    void onOk( void ) ;

  private slots :

    /**
    * @brief Action to be executed when user press the draw (toolbar) button
    */
    void onClickDrawBtn( void ) ;

    /**
    * @brief Action to be executed when user press the erase (toolbar) button
    */
    void onClickEraseBtn( void ) ;

    /**
    * @brief Action to be executed when user press the fill button
    */
    void onClickFillBtn( void ) ;

    /**
    * @brief Action to be executed when user press the clear button
    */
    void onClickClearBtn( void ) ;

    /**
    * @brief Action to be executed when user click on activation checkbox
    */
    void onChangeActivation( void ) ;

    /**
    * @brief Action to be executed when user change brush size slider 
    */
    void onChangeBrushSize( void ) ; 

  private:

    void buildInterface() ;
    void makeConnections() ;

    std::shared_ptr<Project> m_project ;
    int m_image_id ;

    QCheckBox * m_activated ;

    QToolBar * m_drawing_toolbar ;
    QAction * m_draw_act ;
    QAction * m_erase_act ;
    QAction * m_fill_all_act ;
    QAction * m_clear_all_act ;
    QSlider * m_slider_element_size ;

    MaskView * m_drawing_area ;
    QGraphicsScene * m_drawing_scene ;

    QPushButton * m_ok_btn ;
    QPushButton * m_cancel_btn ;

    Q_OBJECT
} ;

} // namespace openMVG_gui

#endif
