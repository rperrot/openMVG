#ifndef _DEPTH_MAP_COMPUTATION_CPU_COST_METRIC_FUNCTOR_HH_
#define _DEPTH_MAP_COMPUTATION_CPU_COST_METRIC_FUNCTOR_HH_

#include "Image.hpp"
#include "DepthMapComputationParameters.hpp"


namespace MVS
{

/**
 * @brief Base class for Cost metric aggregation score
 */
class AbstractCostMetric
{
  public:

    /**
     * @brief Ctr
     * @param img_ref Reference image
     * @param img_other Target image
     * @param params Various parameters of the functor
     */
    AbstractCostMetric( const Image & img_ref , const Image & img_other , const DepthMapComputationParameters & params ) ;

    /**
     * @brief Compute aggregated matching cost at a given pixel
     * @param id_row Row index of the queried pixel
     * @param id_col Col index of the queried pixel
     * @param H Homography that maps pixels from Reference image to the Target image
     * @return Aggregated matching cost at specified pixel
     */
    virtual double operator()( const int id_row , const int id_col , const openMVG::Mat3 & H ) const = 0 ;

  protected:

    const Image & m_image_ref ;
    const Image & m_image_other ;
    const DepthMapComputationParameters & m_params ;
} ;

/**
 * @brief Zero-mean Normalized Cross Correlation cost metric
 */
class ZNCCCostMetric : public AbstractCostMetric
{
  public:

    /**
     * @brief Ctr
     * @param img_ref Reference image
     * @param img_other Target image
     * @param params Various parameters of the functor
     */
    ZNCCCostMetric( const Image & img_ref , const Image & img_other , const DepthMapComputationParameters & params ) ;

    /**
     * @brief Compute aggregated matching cost at a given pixel
     * @param id_row Row index of the queried pixel
     * @param id_col Col index of the queried pixel
     * @param H Homography that maps pixels from Reference image to the Target image
     * @return Aggregated matching cost at specified pixel
     */
    double operator()( const int id_row , const int id_col , const openMVG::Mat3 & H ) const override ;
} ;

/**
 * Patch-Match cost metric
 */
class PatchMatchCostMetric : public AbstractCostMetric
{
  public:

    /**
     * @brief Ctr
     * @param img_ref Reference image
     * @param img_other Target image
     * @param params Various parameters of the functor
     */
    PatchMatchCostMetric( const Image & img_ref , const Image & img_other , const DepthMapComputationParameters & params ) ;

    /**
     * @brief Compute aggregated matching cost at a given pixel
     * @param id_row Row index of the queried pixel
     * @param id_col Col index of the queried pixel
     * @param H Homography that maps pixels from Reference image to the Target image
     * @return Aggregated matching cost at specified pixel
     */
    double operator()( const int id_row , const int id_col , const openMVG::Mat3 & H ) const override ;
} ;

/**
 * @brief Census cost metric
 */
class CensusCostMetric : public AbstractCostMetric
{
  public:

    /**
    * @brief Ctr
    * @param img_ref Reference image
    * @param img_other Target image
    * @param params Various parameters of the functor
    */
    CensusCostMetric( const Image & img_ref , const Image & img_other , const DepthMapComputationParameters & params ) ;

    /**
    * @brief Compute aggregated matching cost at a given pixel
    * @param id_row Row index of the queried pixel
    * @param id_col Col index of the queried pixel
    * @param H Homography that maps pixels from Reference image to the Target image
    * @return Aggregated matching cost at specified pixel
    */
    double operator()( const int id_row , const int id_col , const openMVG::Mat3 & H ) const override ;
} ;

} // namespace MVS

#endif