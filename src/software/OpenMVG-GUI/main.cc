// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2017 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ApplicationSettings.hh"
#include "MainWindow.hh"

#include "workers/WorkerNextAction.hh"

#include <QApplication>
#include <QFontDatabase>
#include <QLocale>
#include <QStandardPaths>

#include <clocale>
#include <string>

using namespace openMVG_gui;

int main( int argc, char **argv )
{
  // OpenGL
  // OpenGL use 3.2 minimum
  QSurfaceFormat format = QSurfaceFormat::defaultFormat();
  format.setVersion( 3, 2 );
  format.setProfile( QSurfaceFormat::CoreProfile );
  QSurfaceFormat::setDefaultFormat( format );

  QApplication app( argc, argv );
  setlocale( LC_ALL, "C" );
  setlocale( LC_NUMERIC, "C" );

  QLocale locale( QLocale::C );
  QLocale::setDefault( locale );

  qRegisterMetaType<openMVG_gui::WorkerNextAction>( "WorkerNextAction" );
  qRegisterMetaType<std::string>( "std::string" );

  // Fonts
  QFontDatabase::addApplicationFont( ":/fonts/SourceCodeVariable-Roman.otf" );

  // Application Path - Create it at the first time
  const std::string appDirPath = QStandardPaths::writableLocation( QStandardPaths::AppDataLocation ).toStdString();

  if ( !stlplus::folder_exists( appDirPath ) )
  {
    stlplus::folder_create( appDirPath );

    if ( !stlplus::file_exists( ApplicationSettings::configPath() ) )
    {
      ApplicationSettings default_settings;
      default_settings.save( ApplicationSettings::configPath() );
    }
  }
  // Load the configuration
  ApplicationSettings::instance() = ApplicationSettings( ApplicationSettings::configPath() );

  MainWindow win;
  win.show();

  return app.exec();
}