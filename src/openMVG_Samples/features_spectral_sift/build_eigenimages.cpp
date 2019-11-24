#include <iostream>

#include <openMVG/numeric/special_functions.hpp>

#include "third_party/cmdLine/cmdLine.h"

using namespace openMVG::numeric;

int main( int argc, char** argv )
{
  CmdLine cmd;

  int         iOrder       = 3;  // Order of the polynomial used to approximate the eigenfunctions.
  int         iFilterWidth = 33; // Size of the eigenimage ( iFilterWidth x iFilterWidth )
  std::string sDirectoryOutput;

  cmd.add( make_option( 'o', iOrder, "Polynomial order" ) );
  cmd.add( make_option( 'n', iFilterWidth, "Filter width" ) );
  cmd.add( make_option( 'd', sDirectoryOutput, "Directory in which eigen_images are written" ) );

  std::cout << "Gamma[-1,0.2,0.3] = " << generalized_incomplete_gamma( 2.0 , 0.2 , 0.3 ) << std::endl; 

  return EXIT_SUCCESS;
}