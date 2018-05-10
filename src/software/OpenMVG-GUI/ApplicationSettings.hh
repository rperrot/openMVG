#ifndef _OPENMVG_SOFTWARE_OPENMVG_GUI_APPLICATION_SETTINGS_HH_
#define _OPENMVG_SOFTWARE_OPENMVG_GUI_APPLICATION_SETTINGS_HH_

#include "openMVG/numeric/numeric.h"

#include <memory>
#include <string>

namespace openMVG_gui
{

/**
 * @brief Class used to handle application settings
 */
class ApplicationSettings
{
public:
  static const openMVG::Vec4 DEFAULT_VIEW_BACKGROUND_COLOR;
  // Get the instance of the application settings
  static ApplicationSettings &instance();
  // Get the path of the application configuration file
  static std::string configPath( void );

  /**
   * Default settings
   */
  ApplicationSettings( void );

  /**
   * @brief Settings from a file
   * @param file File to load
   */
  ApplicationSettings( const std::string &file );

  ApplicationSettings( const ApplicationSettings &src ) = default;
  ApplicationSettings( ApplicationSettings &&src )      = default;

  ApplicationSettings &operator=( const ApplicationSettings &src ) = default;
  ApplicationSettings &operator=( ApplicationSettings &&src ) = default;

  /**
   * @brief Get background color of the 3d view
   * @return background color
   */
  openMVG::Vec4 viewBackgroundColor( void ) const;

  /**
   * @brief Set background color of the 3d view
   * @param col New background color
   */
  void setViewBackgroundColor( const openMVG::Vec4 &col );

  /**
   * @brief Save application settings to a file
   * @param file The output file
   * @retval true if save is OK
   * @retval false if save fails
   */
  bool save( const std::string &file );

  void set( const ApplicationSettings & src);

private:
  /**
   *  Load a settings file
   * @param file File to load
   * @retval true if load is OK
   * @retval false if load fails
   */
  bool load( const std::string &file );

  /// The view background color (RGBA in range [0-1])
  openMVG::Vec4 m_view_background_color;
};

} // namespace openMVG_gui

#endif