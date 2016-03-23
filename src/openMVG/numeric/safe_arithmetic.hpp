#ifndef OPENMVG_SAFE_ARITHMETIC_HPP_
#define OPENMVG_SAFE_ARITHMETIC_HPP_

#include <type_traits>
#include <limits>

namespace openMVG
{

/**
* @brief handle integer overflow when using arithmetic addition 
* @param a Integer value 
* @param b Integer value 
* @retval std::numeric_limits<T>::max() if positive overflow occurs 
* @retval std::numeric_limits<T>::min() if negative overflow occurs 
* @retval a + b if no overflow occurs  
*/
template< typename T >
static inline typename std::enable_if< std::is_integral<T>::value , T>::type SafeAdd( const T a , const T b )
{
  const bool positive = b >= 0 ;
  const bool overflow = positive ?
                        a > std::numeric_limits<T>::max() - b : /* positive Overflow */
                        a < std::numeric_limits<T>::min() - b ; /* nefative underflow */
  return overflow ?
         ( positive ? std::numeric_limits<T>::max() : std::numeric_limits<T>::min() ) : /* Overflow */
         ( a + b ) ; /* None */
}

/**
* @brief Compute safe sum 
* @param a a value 
* @param b a value 
* @return a + b
*/
template< typename T >
static inline typename std::enable_if < !std::is_integral<T>::value , T >::type SafeAdd( const T a , const T b )
{
  return a + b ;
}



}

#endif