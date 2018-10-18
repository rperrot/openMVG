#include "DownloadFileManager.hh"

#include <QEventLoop>

#include <iostream>

namespace openMVG_gui
{
DownloadFileManager::DownloadFileManager( const std::string &url,
                                          const std::string &path ) :
    m_url( url.c_str() ),
    m_outPath( path.c_str() )
{
  setup();
}

bool DownloadFileManager::download( void )
{
  if ( !m_success )
  {
    // Output file cannot be openned
    return false;
  }
  QNetworkRequest request( m_url );
  request.setAttribute( QNetworkRequest::FollowRedirectsAttribute, true );
  m_reply = m_mgr.get( request );
  connect( m_reply, SIGNAL( readyRead() ), this, SLOT( readyToRead() ) );

  QEventLoop loop;
  connect( m_reply, SIGNAL( finished() ), &loop, SLOT( quit() ) );
  loop.exec();
  m_outFile.close();

  return m_success;
}

void DownloadFileManager::readyToRead()
{
  QByteArray data = m_reply->readAll();
  m_outFile.write( data );
}

void DownloadFileManager::finished( QNetworkReply *reply )
{
  m_success = reply->error() == QNetworkReply::NoError;

  if ( m_success )
  {
    QByteArray data = reply->readAll();
    m_outFile.write( data );
  }

  reply->deleteLater();
}

void DownloadFileManager::setup()
{
  connect( &m_mgr, SIGNAL( finished( QNetworkReply * ) ), this,
           SLOT( finished( QNetworkReply * ) ) );

  m_outFile.setFileName( m_outPath.c_str() );
  m_success = m_outFile.open( QIODevice::ReadWrite );
}
} // namespace openMVG_gui