#include "MainWindow.hh"

#include "workers/WorkerNextAction.hh"

#include <QApplication>

#include <clocale>
#include <string>

using namespace openMVG_gui ;

int main( int argc , char ** argv )
{
  // OpenGL
  // OpenGL use 3.2 minimum
  QSurfaceFormat format = QSurfaceFormat::defaultFormat() ;
  format.setVersion( 3 , 2 );
  format.setProfile( QSurfaceFormat::CoreProfile );
  QSurfaceFormat::setDefaultFormat( format );

  QApplication app( argc , argv ) ;
  setlocale( LC_ALL, "C" ) ;
  setlocale( LC_NUMERIC, "C" ) ;

  qRegisterMetaType<openMVG_gui::WorkerNextAction>( "WorkerNextAction" );
  qRegisterMetaType<std::string>( "std::string" ) ;



  MainWindow win ;
  win.show() ;

  return app.exec() ;
}