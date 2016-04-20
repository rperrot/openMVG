#include <QApplication>

#include "software/SfMGui/MainWindow.hpp"

using namespace openMVG::SfMGui ;

int main( int argc , char ** argv )
{
  QApplication app( argc , argv ) ;

  MainWindow win ;
  win.show() ;

  return app.exec() ;
}