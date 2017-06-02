#include "MainWindow.hh"

#include "workers/WorkerNextAction.hh"

#include <GL/glew.h>

#include <QApplication>

#include <clocale>

using namespace openMVG_gui ;

int main( int argc , char ** argv )
{
  QApplication app( argc , argv ) ;
  setlocale( LC_ALL, "C" ) ;
  setlocale( LC_NUMERIC, "C" ) ;

  qRegisterMetaType<openMVG_gui::WorkerNextAction>( "WorkerNextAction" );

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