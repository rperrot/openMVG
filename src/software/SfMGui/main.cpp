#include <QApplication>

#include "software/SfMGui/MainWindow.hpp"
#include "software/SfMGui/SfMProject.hpp"

#include <memory>
#include <clocale>

using namespace openMVG::SfMGui ;

Q_DECLARE_METATYPE( std::shared_ptr<SfMProject> )

int main( int argc , char ** argv )
{
  QApplication app( argc , argv ) ;

  qRegisterMetaType<std::shared_ptr<SfMProject> >();


  MainWindow win ;

  setlocale( LC_ALL , "C" ) ;

  win.show() ;

  return app.exec() ;
}