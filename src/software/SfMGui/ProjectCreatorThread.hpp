#ifndef _PROJECT_CREATOR_THREAD_HPP_
#define _PROJECT_CREATOR_THREAD_HPP_

#include "software/SfMGui/SfMProject.hpp"

#include <QObject>
#include <QThread>

#include <memory>

namespace openMVG
{
namespace SfMGui
{
/**
* @brief Simple worker that create a scene
* @note User should not use this class, prefer ProjectCreator instead
*/
class ProjectCreatorThreadWorker : public QObject
{
    Q_OBJECT

  public:

    /**
    * @brief ctr
    * @param output_project The output project path
    * @param input_image The input image path
    */
    ProjectCreatorThreadWorker( const std::string & output_project , const std::string & input_image ) ;

  public slots:

    /**
    * @brief start creation of the project
    */
    void process( void ) ;

  signals:

    /**
    * @brief signal emitted when project is created
    * @param pro the newly created project
    */
    void finished( std::shared_ptr< SfMProject > pro ) ;

  private:
    std::string m_output_project_folder_path ;
    std::string m_input_image_folder_path ;
} ;

/**
* @brief class creating a project in a multithread way
*/
class ProjectCreator : public QObject
{
    Q_OBJECT

  public:

    /**
    * @brief Constructor
    * @param output_project_path The output path of the project
    * @param input_image_path The input path of the project
    */
    ProjectCreator( const std::string & output_project_path , const std::string input_image_path ) ;

    ~ProjectCreator();

  public slots :

    /**
    * @brief Start creation of the project
    */
    void start( void ) ;

    void hasFinished( std::shared_ptr< SfMProject > pro ) ;

  signals:

    /**
    * @brief signal emitted when project is created
    * @param pro the newly created project
    */
    void finished( std::shared_ptr< SfMProject > pro ) ;

  private:

    ProjectCreatorThreadWorker * m_worker ;
    QThread m_thread ;

} ;

} // namespace SfMGui
} // namespace openMVG

#endif