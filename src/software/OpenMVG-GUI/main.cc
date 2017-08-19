#include "MainWindow.hh"

#include "workers/WorkerNextAction.hh"

#include <QApplication>

#include <clocale>
#include <string>

using namespace openMVG_gui ;

int main( int argc , char ** argv )
{
  QApplication app( argc , argv ) ;
  setlocale( LC_ALL, "C" ) ;
  setlocale( LC_NUMERIC, "C" ) ;

  qRegisterMetaType<openMVG_gui::WorkerNextAction>( "WorkerNextAction" );
  qRegisterMetaType<std::string>( "std::string" ) ;

  // OpenGL
  // OpenGL use 3.3 minimum
  QSurfaceFormat format;
  format.setVersion( 3 , 2 );
  format.setProfile( QSurfaceFormat::CoreProfile );
  QSurfaceFormat::setDefaultFormat( format );


  MainWindow win ;
  win.show() ;

  return app.exec() ;
}