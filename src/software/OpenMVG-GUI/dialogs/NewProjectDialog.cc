// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2017 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "NewProjectDialog.hh"

#include "third_party/stlplus3/filesystemSimplified/file_system.hpp"

#include <QFileDialog>
#include <QGridLayout>
#include <QMessageBox>
#include <QVBoxLayout>

namespace openMVG_gui
{

/**
* @brief Ctr
*/
NewProjectDialog::NewProjectDialog( QWidget * parent )
  : QDialog( parent )
{
  buildInterface() ;
  makeConnections() ;
  setWindowTitle( "New project" ) ;
  setModal( true ) ;
  show();
}

/**
* @brief Path of the output project
*/
std::string NewProjectDialog::outputProjectPath( void ) const
{
  QString value = m_line_project->text( ) ;
  if( value.isEmpty() || value.isNull() )
  {
    return std::string() ;
  }
  else
  {
    return value.toStdString() ;
  }
}

/**
* @brief Path of the input image
*/
std::string NewProjectDialog::inputImagePath( void ) const
{
  QString value = m_line_input->text( ) ;
  if( value.isEmpty() || value.isNull() )
  {
    return std::string() ;
  }
  else
  {
    return value.toStdString() ;
  }
}

/**
* @brief action to be executed when user want to select output folder
*/
void NewProjectDialog::onWantToSelectProjectPath( void )
{
  QString dir =  QFileDialog::getExistingDirectory( this, tr( "Select project directory" ),
                 QDir::homePath() ,
                 QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks );
  if( ! ( dir.isEmpty() || dir.isNull() ) )
  {
    m_line_project->setText( dir ) ;
  }
}

/**
* @brief action to be executed when user want to select input image folder
*/
void NewProjectDialog::onWantToSelectImagePath( void )
{
  QString dir =  QFileDialog::getExistingDirectory( this, tr( "Select input image directory" ),
                 QDir::homePath() ,
                 QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks );
  if( ! ( dir.isEmpty() || dir.isNull() ) )
  {
    m_line_input->setText( dir ) ;
  }
}

/**
* @brief action to be executed when user click on cancel button
*/
void NewProjectDialog::onCancel( void )
{
  done( QDialog::Rejected ) ;
}

/**
* @brief action to be executed when user click on ok button
*/
void NewProjectDialog::onOk( void )
{
  const std::string p_path = outputProjectPath() ;
  const std::string i_path = inputImagePath() ;

  bool projectPathValid = stlplus::folder_exists( p_path ) ;
  bool inputPathValid = stlplus::folder_exists( i_path ) ;
  bool valid = projectPathValid && inputPathValid ;
  if( valid )
  {
    done( QDialog::Accepted ) ;
  }
  else
  {
    if( ! projectPathValid )
    {
      QMessageBox::critical( this , "Error" , "Project path is invalid" ) ;
    }
    else
    {
      QMessageBox::critical( this , "Error" , "Input image path is invalid" ) ;
    }
  }
}

void NewProjectDialog::buildInterface( void )
{
  QGridLayout * glayout = new QGridLayout ;

  m_lbl_input_image = new QLabel( "Input folder" ) ;
  m_lbl_output_project = new QLabel( "Output folder" ) ;

  m_line_input = new QLineEdit( this ) ;
  m_line_input->setEnabled( false ) ;
  m_line_project = new QLineEdit( this ) ;
  m_line_project->setEnabled( false ) ;

  m_btn_project = new QPushButton( "..." , this ) ;
  m_btn_project->setDefault( false );
  m_btn_image = new QPushButton( "..." , this ) ;
  m_btn_image->setDefault( false ) ;

  glayout->addWidget( m_lbl_input_image , 0 , 0 ) ;
  glayout->addWidget( m_line_input , 0 , 1 ) ;
  glayout->addWidget( m_btn_image , 0 , 2 ) ;
  glayout->addWidget( m_lbl_output_project , 1 , 0 ) ;
  glayout->addWidget( m_line_project , 1 , 1 ) ;
  glayout->addWidget( m_btn_project , 1 , 2 ) ;

  m_btn_cancel = new QPushButton( "Cancel" ) ;
  m_btn_cancel->setDefault( false ) ;
  m_btn_ok = new QPushButton( "OK" ) ;
  m_btn_ok->setDefault( true ) ;

  QHBoxLayout * btnLayout = new QHBoxLayout ;
  btnLayout->addStretch() ;
  btnLayout->addWidget( m_btn_cancel ) ;
  btnLayout->addWidget( m_btn_ok ) ;

  QVBoxLayout * mainLayout = new QVBoxLayout ;

  mainLayout->addLayout( glayout ) ;
  mainLayout->addLayout( btnLayout ) ;

  setLayout( mainLayout ) ;
  adjustSize();
  mainLayout->setSizeConstraint( QLayout::SetFixedSize );

}

void NewProjectDialog::makeConnections( void )
{
  connect( m_btn_image , SIGNAL( clicked() ) , this , SLOT( onWantToSelectImagePath() ) ) ;
  connect( m_btn_project , SIGNAL( clicked() ) , this , SLOT( onWantToSelectProjectPath() ) ) ;
  connect( m_btn_cancel , SIGNAL( clicked() ) , this , SLOT( onCancel() ) )  ;
  connect( m_btn_ok , SIGNAL( clicked() ) , this , SLOT( onOk() ) ) ;
}

} // namespace openMVG_gui
