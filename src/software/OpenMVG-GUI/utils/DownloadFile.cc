#include "DownloadFile.hh"

#include "DownloadFileManager.hh"

#include <QDir>
#include <QFile>
#include <QNetworkAccessManager>
#include <QStandardPaths>
#include <QUrl>

#include <mutex>

namespace openMVG_gui
{

/**
 * @brief Download a file from an URL
 *
 * @param base_url    URL of the file to download
 * @param output_path Path where the file will be downloaded
 * @param hash_url    URL of the hash file
 *
 * @retval true if success
 * @retval false if failure
 *
 * @note if no hash URL is given, no hash is computed
 * @note HASH used is SHA256
 */
bool DownloadFile( const std::string &base_url, const std::string &output_path, const std::string &hash_url )
{
  DownloadFileManager mgrFile( base_url, output_path );
  if ( !mgrFile.download() )
  {
    return false;
  }
  if ( hash_url != "" )
  {
    const QString tmp_path =
        QStandardPaths::writableLocation( QStandardPaths::AppDataLocation ) + QDir::separator() + "tmp.sha256";
    DownloadFileManager mgrSha( hash_url, tmp_path.toStdString() );
  }

  return true;
}

} // namespace openMVG_gui