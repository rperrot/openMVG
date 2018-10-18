#ifndef _DOWNLOAD_FILE_MANAGER_HH_
#define _DOWNLOAD_FILE_MANAGER_HH_

#include <QFile>
#include <QNetworkReply>
#include <QObject>
#include <QUrl>

#include <string>

namespace openMVG_gui
{

class DownloadFileManager : public QObject
{
public:
  DownloadFileManager( const std::string &url, const std::string &path );

  bool download( void );

private slots:

  void readyToRead(  );

  void finished( QNetworkReply *reply );

private:
  void setup( void );

  bool        m_success;
  QUrl        m_url;
  std::string m_outPath;
  QFile       m_outFile;

  QNetworkAccessManager m_mgr;
  QNetworkReply *       m_reply;

  Q_OBJECT
};

} // namespace openMVG_gui

#endif