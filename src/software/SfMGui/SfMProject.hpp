#ifndef _OPENMVG_SFMGUI_SFM_PROJECT_HH_
#define _OPENMVG_SFMGUI_SFM_PROJECT_HH_

#include <string>

namespace openMVG
{
namespace SfMGui
{
/**
* @brief class managing a SfM project in GUI
*/
class SfMProject
{
  public:


    /**
    * @brief Save project
    * @retval true if save can be done
    * @retval false if save can't be done
    */
    bool Save( void ) ;

    /**
    * @brief Save project as
    * @retval true if save can be done
    */
    bool SaveAs( const std::string & path ) ;



  private:

    /**
    * Folder
    * Folder/gui/thumbnail
    * Folder/internal/
    * Folder/internal/sfm_data/...
    */
    void BuildProjectStructure( void ) ;

    /// Root project folder
    std::string m_project_folder ;

    /// Project file
    std::string m_project_file ;

} ;
}
}

#endif