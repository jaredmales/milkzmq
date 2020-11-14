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

#include <ImageStreamIO.h>

#include <xrif/xrif.h>
namespace milkzmq 
{

   
//The milkzmq messager format:
/*
 *  0-127    image stream name
 *  128      data type code (uint8_t)
 *  129-141  size 0 (uint32_t)
   
 */
constexpr size_t headerSize = 256; ///< total size in bytes of the header,  gives room to grow!

constexpr size_t nameSize = 128; ///< The size of the name field.
constexpr size_t typeOffset = nameSize;                             ///< Start of data type field
constexpr size_t size0Offset = typeOffset+sizeof(uint8_t);     ///< start of size0 field
constexpr size_t size1Offset = size0Offset + sizeof(uint32_t); ///< start of size1 field
constexpr size_t cnt0Offset = size1Offset + sizeof(uint32_t);
constexpr size_t tv_secOffset = cnt0Offset + sizeof(uint64_t);
constexpr size_t tv_nsecOffset = tv_secOffset + sizeof(uint64_t);
constexpr size_t xrifDifferenceOffset = tv_nsecOffset + sizeof(uint64_t); ///< The XRIF encoding differencing method.  Generally must be PIXEL.
constexpr size_t xrifReorderOffset = xrifDifferenceOffset + sizeof(int16_t);                  ///< The XRIF encoding reordering method.
constexpr size_t xrifCompressOffset = xrifReorderOffset + sizeof(int16_t);                 ///< The XRIF encoding compression method.
constexpr size_t xrifSizeOffset =  xrifCompressOffset + sizeof(int16_t);                    ///< The size of the compressed data.

constexpr size_t endOfHeader = xrifSizeOffset + sizeof(uint16_t);         ///< The current end of the header.
constexpr size_t imageOffset = headerSize;
      
static_assert(endOfHeader <= imageOffset, "Header fields sum to larger than reserved headerSize");

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

/// Report status (with LOG_INFO level of priority) to the user using stderr.
inline 
void reportInfo( const std::string & argv0, ///< [in] the name of the application reporting status
                 const std::string & msg    ///< [in] the status message
               )
{
   std::cerr << argv0  <<": " << msg << "\n";
}

/// Report status (with LOG_NOTICE level of priority)  to the user using stderr.
inline 
void reportNotice( const std::string & argv0, ///< [in] the name of the application reporting status
                   const std::string & msg    ///< [in] the status message
                 )
{
   std::cerr << argv0  <<": " << msg << "\n";
}

/// Report a warning to the user using stderr.
inline 
void reportWarning( const std::string & argv0, ///< [in] the name of the application reporting the warning
                    const std::string & msg    ///< [in] the warning message
                  )
{
   std::cerr << argv0  <<": " << msg << "\n";
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

///Global needed for ImageStreamIO error reporting.
std::string milkzmq_argv0;

/// ImageStreamIO error reporting function to pass to library.  
errno_t milkzmq_printError( const char *file, const char *func, int line, errno_t code, char *errmessage )
{
   
   std::string msg = "ImageStreamIO (";
   msg += func;
   msg += ") Error Msg: ";
   msg += errmessage;
   msg += " [code: ";
   msg += std::to_string(code);
   msg += "]";
   
   reportError(milkzmq_argv0, msg, file, line);
   
   return IMAGESTREAMIO_SUCCESS;
}

} //namespace milkzmq 

#endif //milkZeroUtils_hpp
