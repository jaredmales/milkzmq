/** \file milkzmqUtils.hpp
  * \brief Useful tools for milk 0.
  * \author Jared R. Males (jaredmales@gmail.com)
  *
  * History:
  * - 2018-12-27 created by JRM
  */

//***********************************************************************//
// Copyright 2018 Jared R. Males (jaredmales@gmail.com)
//
// This file is part of milkzmq.
//
// milkzmq is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// milkzmq is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with milkzmq.  If not, see <http://www.gnu.org/licenses/>.
//***********************************************************************//

#ifndef milkzmqUtils_hpp
#define milkzmqUtils_hpp

#include <iostream>
#include <string>
#include <chrono>
#include <thread>

namespace milkzmq 
{

constexpr size_t typeOffset = 128;
constexpr size_t size0Offset = typeOffset+sizeof(uint8_t);
constexpr size_t size1Offset = size0Offset + sizeof(uint32_t);
constexpr size_t cnt0Offset = size1Offset + sizeof(uint32_t);
constexpr size_t tv_secOffset = cnt0Offset + sizeof(uint64_t);
constexpr size_t tv_nsecOffset = tv_secOffset + sizeof(uint64_t);
constexpr size_t imageOffset = tv_nsecOffset + sizeof(uint64_t);
      
/// Sleep for a specified period in seconds.
inline
void sleep( unsigned sec /**< [in] the number of seconds to sleep. */)
{
   std::this_thread::sleep_for(std::chrono::seconds(sec));
}

/// Sleep for a specified period in microseconds.
inline
void microsleep( unsigned usec /**< [in] the number of microseconds to sleep. */)
{
   std::this_thread::sleep_for(std::chrono::microseconds(usec));
}

/// Get the current time, as double precision seconds since the epoch
/** 
  * \returns the time since the epoch.
  */ 
inline
double get_curr_time()
{
   struct timespec tsp;
   clock_gettime(CLOCK_REALTIME, &tsp);

   return ((double)tsp.tv_sec) + ((double)tsp.tv_nsec)/1e9;
}

/// Report an error to the user using stderr.
inline 
void reportError( const std::string & argv0, ///< [in] the name of the application reporting the error
                  const std::string & msg,   ///< [in] the error message
                  const std::string & file,  ///< [in] the file where the error occurred
                  int line                   ///< [in] the line number at which the error occurred.
                )
{
   std::cerr << argv0  <<": " << msg << "\n";
   std::cerr << "  at " << file << " line " << line << "\n"; 
}

/// Report a warning to the user using stderr.
inline 
void reportWarning( const std::string & argv0, ///< [in] the name of the application reporting the warning
                    const std::string & msg    ///< [in] the warning message
                  )
{
   std::cerr << argv0  <<": " << msg << "\n";
}

} //namespace milkzmq 

#endif //milkZeroUtils_hpp
