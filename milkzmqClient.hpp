/** \file milkzmqClient.hpp
  * \brief Class implementing a ZeroMQ ImageStreamIO client
  * \author Jared R. Males (jaredmales@gmail.com)
  *
  * History:
  * - 2018 created by JRM
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

#ifndef milkzmqClient_hpp
#define milkzmqClient_hpp

#include <zmq.hpp>

#include <ImageStreamIO.h>

#include "milkzmqUtils.hpp"

namespace milkzmq 
{
   
class milkzmqClient
{
protected:
   
   /** \name Configurable Parameters 
     *
     *@{
     */
   
   std::string m_argv0 {"milkzmqClient"}; ///< The invoked name, used for error messages.
   
   std::string m_address {""}; ///< The address of the image server.
   
   int m_imagePort{5556}; ///< The port number to use for the image server.
   
   std::string m_shMemImName; ///< The name of the ImageStreamIO shared memory stream to monitor (and maybe update)
   
   std::string m_localShMemImName; ///< The local shared memory stream name to update.  Optional, if "" then m_shMemImName.
   
   ///@}
   
   /** \name Internal State 
     *
     *@{
     */
   std::thread m_imageThread; ///< Thread for publishing image slice updates

   zmq::context_t * m_ZMQ_context {nullptr}; ///< The ZeroMQ context, allocated on construction.

   ///@}
   
public:
   
   /// Default c'tor
   milkzmqClient();
   
   /// Destructor
   ~milkzmqClient();
   
   /// Set the invoked name of the application.
   /** This sets the value of m_argv0, used for error reporting.
     * 
     * \returns 0 on success
     * \returns -1 on error
     */
   int argv0( const std::string & av0 /**< [in] the new invoked name */);
   
   /// Get the invoked name
   /**
     * \returns the invoked name, the current value of m_argv0
     */
   std::string argv0();
   
   /// Set the address of the remote server.
   /** This sets the value of m_address.
     * 
     * \returns 0 on success
     * \returns -1 on error
     */
   int address( const std::string & add /**< [in] the new address */ );
   
   /// Get the address
   /**
     * \returns the remote address, the current value of m_address
     */
   std::string address();
   
   /// Set the port number of the image server
   /** This sets the value of m_imagePort.
     * 
     * \returns 0 on success
     * \returns -1 on error
     */ 
   int imagePort( const int & name /**< [in] the new image port number*/);
   
   /// Get the image port number
   /**
     * \returns the image port number, the current value of m_imagePort
     */ 
   int imagePort();
   
   /// Set the name of the ImageStreamIO shared memory image
   /** This is just the root.  E.g. for a complete path of '/tmp/image00.im.shm' the argument should be "image00".
     * 
     * \returns 0 on success
     * \returns -1 on error
     */ 
   int shMemImName( const std::string & name /**< [in] the  new name of the shared memory image.*/);
   
   /// Get the name of the shared memory image.
   /**
     * \returns the name of the shared memory image, the current value of m_shMemImName.
     */ 
   std::string shMemImName();
   
   /// Set the name of the local ImageStreamIO shared memory image
   /** This is just the root.  E.g. for a complete path of '/tmp/image00.im.shm' the argument should be "image00".
     * 
     * \returns 0 on success
     * \returns -1 on error
     */ 
   int localShMemImName( const std::string & name /**< [in] the  new name of the shared memory image.*/);
   
   /// Get the name of the local shared memory image.
   /**
     * \returns the name of the local shared memory image, the current value of m_localShMemImName.
     */ 
   std::string localShMemImName();
   
private:
   ///Thread starter, called by imageThreadStart on thread construction.  Calls imageThreadExec.
   static void internal_imageThreadStart( milkzmqClient * mzs /**< [in] a pointer to a milkzmqClient instance (normally this) */);

public:
   /// Start the image thread.
   int imageThreadStart();

   /// Execute the image thread.
   void imageThreadExec();

   /// Flag to control execution.  When true all threads will exit.
   static bool m_timeToDie;
   
   /** \name Error Handling
     * Errors are reported using a virtual function, so that custom handling can be implemented.
     *
     * @{
     */
   virtual void reportError( const std::string & msg,
                             const std::string & file,
                             int line
                           );
   ///@}
};

bool milkzmqClient::m_timeToDie = false;

inline
milkzmqClient::milkzmqClient()
{
   m_ZMQ_context = new zmq::context_t(1);
}

inline
milkzmqClient::~milkzmqClient()
{
   m_timeToDie = true;
   
   if(m_imageThread.joinable()) 
   {
      pthread_kill(m_imageThread.native_handle(), SIGTERM);
      m_imageThread.join();
   }
   if(m_ZMQ_context) delete m_ZMQ_context;
}

inline 
int milkzmqClient::argv0( const std::string & av0 )
{
   m_argv0 = av0;
   return 0;
}

inline 
std::string milkzmqClient::argv0()
{
   return m_argv0;
}

inline 
int milkzmqClient::address( const std::string & add )
{
   m_address= add;
   return 0;
}

inline 
std::string milkzmqClient::address()
{
   return m_address;
}

inline
int milkzmqClient::imagePort( const int & imagePort )
{
   m_imagePort = imagePort;
   
   return 0;
}

inline
int milkzmqClient::imagePort()
{
   return m_imagePort;
}

inline
int milkzmqClient::shMemImName( const std::string & name )
{
   m_shMemImName = name;
   
   return 0;
}

inline
std::string milkzmqClient::shMemImName()
{
   return m_shMemImName;
}

inline
int milkzmqClient::localShMemImName( const std::string & name )
{
   m_localShMemImName = name;
   
   return 0;
}

inline
std::string milkzmqClient::localShMemImName()
{
   return m_localShMemImName;
}

inline
void milkzmqClient::internal_imageThreadStart( milkzmqClient * mzc )
{
   mzc->imageThreadExec();
}

inline
int milkzmqClient::imageThreadStart()
{
   try
   {
      m_imageThread = std::thread( internal_imageThreadStart, this);
   }
   catch( const std::exception & e )
   {
      reportError(std::string("exception in image thread startup: ") + e.what(), __FILE__, __LINE__);
      return -1;
   }
   catch( ... )
   {
      reportError("unknown exception in image thread startup" , __FILE__, __LINE__);
      return -1;
   }
   
   if(!m_imageThread.joinable())
   {
      reportError("image thread did not start" , __FILE__, __LINE__);
      return -1;      
   }
   
   return 0;
}

inline
void milkzmqClient::imageThreadExec()
{   
   //size_t type_size = 0; ///< The size, in bytes, of the image data type

   std::string srvstr = "tcp://" + m_address + ":" + std::to_string(m_imagePort);
   
   std::cout << "milkzmqClient: Beginning receive at " << srvstr << "\n";
   
   zmq::socket_t subscriber (*m_ZMQ_context, ZMQ_SUB);
   

   subscriber.connect(srvstr);
   
   std::cerr << "connected\n";
   char filter[128];
   memset(filter, 0, 128);
   snprintf(filter, 128, "%s", m_shMemImName.c_str());
   subscriber.setsockopt(ZMQ_SUBSCRIBE, filter, strlen (filter));
   
   
   std::string shMemImName;
   if(m_localShMemImName == "") shMemImName = m_shMemImName;
   else shMemImName = m_localShMemImName;
   
   uint8_t new_atype, atype=0;
   uint64_t new_nx, nx =0;
   uint64_t new_ny, ny =0;
   
   /* Initialize ImageStreamIO
    */
   
   IMAGE image;
   bool opened = false;
    
   uint32_t imsize[3];
    
   int curr_image;
   
   while(!m_timeToDie)
   {
      zmq::message_t msg;
        
      try
      {
         subscriber.recv(&msg);
      }
      catch(...)
      {
         if(m_timeToDie) return; //This will be if signaled during shutdown
         throw; //otherwise uh-oh
      }

      char * raw_image= (char *) msg.data();
      
      new_atype = *( (uint8_t *) (raw_image + typeOffset) );
      new_nx = *( (uint32_t *) (raw_image + size0Offset));
      new_ny = *( (uint32_t *) (raw_image + size1Offset));
      
      if( nx != new_nx || ny != new_ny || atype != new_atype)
      {
         imsize[0] = new_nx;
         imsize[1] = new_ny;
         imsize[2] = 0;
         
         if(opened)
         {
            ImageStreamIO_destroyIm(&image);
         }
         
         ImageStreamIO_createIm(&image, shMemImName.c_str(), 2, imsize, new_atype, 1, 0);
         
         opened = true;
      }
      
      atype = new_atype;
      nx = new_nx;
      ny = new_ny;
      

      //This is not a rolling buffer.
      curr_image = 0;
      
      size_t type_size = ImageStreamIO_typesize(image.md[0].datatype);

      image.md[0].write=1;

      image.md[0].cnt0 = *( (uint64_t *) (raw_image + cnt0Offset));
      image.md[0].atime.tv_sec = *( (uint64_t *) (raw_image + tv_secOffset));
      image.md[0].atime.tv_nsec = *( (uint64_t *) (raw_image + tv_nsecOffset));
      
      
      memcpy(image.array.SI8 + curr_image*nx*ny*type_size, raw_image + imageOffset, nx*ny*type_size);
      
        

      image.md[0].cnt1=0;
      image.md[0].write=0;
      ImageStreamIO_sempost(&image,-1);
   }

   if(opened) ImageStreamIO_closeIm(&image);
   
   
} // milkzmqClient::imageThreadExec()

inline 
void milkzmqClient::reportError( const std::string & msg,
                                  const std::string & file,
                                  int line
                                )
{
   milkzmq::reportError(m_argv0, msg, file, line);
}

} //namesapce milkzmq 

#endif //milkzmqClient_hpp
