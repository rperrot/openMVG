#include <QApplication>

#include "software/SfMGui/MainWindow.hpp"

#include <clocale>

using namespace openMVG::SfMGui ;

int main( int argc , char ** argv )
{
  QApplication app( argc , argv ) ;

  MainWindow win ;

  setlocale( LC_ALL , "C" ) ;

  win.show() ;

  return app.exec() ;
}