#ifndef _OPENMVG_SOFTWARE_OPENMVG_GUI_WORKFLOW_PARAMS_INCREMENTAL_SFM_PARAMS_HH_
#define _OPENMVG_SOFTWARE_OPENMVG_GUI_WORKFLOW_PARAMS_INCREMENTAL_SFM_PARAMS_HH_

#include "IntrinsicParams.hh"
#include "IntrinsicRefinerOptions.hh"

#include <utility>

namespace openMVG_gui
{
/**
* @brief Class holding params for incremental SfM reconstruction
*/
class IncrementalSfMParams
{
  public:

    /**
    * @brief Ctr
    * @param initial_pair Initial pair (if pair is (-1,-1), use automatic selection)
    * @param unknown_intrinsic Intrisic param used for view with unknown intrinsic
    * @param refiner What intrinsics params will be refined by SfM optimizer
    */
    IncrementalSfMParams( const std::pair< int , int > & initial_pair = std::make_pair( -1, -1 ) ,
                          const IntrinsicParams & unknown_intrinsic = INTRINSIC_TYPE_PINHOLE_RADIAL_3 ,
                          const IntrinsicRefinerOptions & refiner = IntrinsicRefinerOptions::INTRINSIC_REFINER_ALL ) ;

    /**
    * @brief Get initial pair
    * @return initial pair
    */
    std::pair<int, int> initialPair( void ) const ;

    /**
    * @brief Set initial pair
    * @param init Initial pair
    */
    void setInitialPair( const std::pair<int, int> & init ) ;

    /**
    * @brief Get intrinsic value for view with unknown intrinsics
    * @return unknown intrinsic params
    */
    IntrinsicParams unknownIntrinsic( void ) const ;

    /**
    * @brief Set intrinsic value for view with unknown intrinsics
    * @param params Intrinsic params to use
    */
    void unknownIntrinsic( const IntrinsicParams & params ) ;

    /**
    * @brief Get refiner options
    * @return refiner options
    */
    IntrinsicRefinerOptions refinerOptions( void ) const ;

    /**
    * @brief Set refiner options
    * @param opts refiner options
    */
    void setRefinerOptions( const IntrinsicRefinerOptions & opts ) ;


    template< class Archive >
    void load( Archive & ar ) ;

    template< class Archive >
    void save( Archive & ar ) const ;

  private:

    std::pair<int, int> m_initial_pair ;
    IntrinsicParams m_unknown_param ;
    IntrinsicRefinerOptions m_refiner_option ;
} ;

template< class Archive >
void IncrementalSfMParams::load( Archive & ar )
{
  ar( m_initial_pair ) ;
  ar( m_unknown_param ) ;
  ar( m_refiner_option ) ;
}

template< class Archive >
void IncrementalSfMParams::save( Archive & ar ) const
{
  ar( m_initial_pair ) ;
  ar( m_unknown_param ) ;
  ar( m_refiner_option ) ;
}

} // namespace openMVG_gui

#endif