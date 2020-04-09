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

#include <signal.h>

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
   
   ///@}
   
   /** \name Internal State 
     *
     *@{
     */
   
   ///Structure to manage the image threads, including startup.
   struct s_imageThread
   {
      std::thread * m_thread {nullptr}; ///< Thread for receiving image slice updates.  A pointer to allow copying, but must be deleted in d'tor of parent.
      milkzmqClient * m_mzc;            ///< a pointer to a milkzmqClient instance (normally this)
      std::string m_imageName;          ///< the name of the image to subscribe from this thread
      std::string m_localImageName;     ///< optional local name of this image stream.  Ignored if "".
      
      ///C'tor to create the thread object
      s_imageThread()
      {
         m_thread = new std::thread;
      }      
   };

   std::vector<s_imageThread> m_imageThreads; ///< The image threads, one per shared memory streamm being served.
   
   //zmq::context_t * m_ZMQ_context {nullptr}; ///< The ZeroMQ context, allocated on construction.
   void * m_ZMQ_context {nullptr};

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
   
   /// Add a ImageStreamIO shared memory image
   /** This is just the root.  E.g. for a complete path of '/tmp/image00.im.shm' the argument should be "image00".
     * This image name is appeneded to the list.
     * 
     * \returns 0 on success
     * \returns -1 on error
     */ 
   int shMemImName( const std::string & name /**< [in] the remote name of a shared memory image.*/);
   
   /// Add a ImageStreamIO shared memory image, along with a local name.
   /** This is just the root.  E.g. for a complete path of '/tmp/image00.im.shm' the argument should be "image00".
     * This image name is appeneded to the list.
     * 
     * \overload
     * 
     * \returns 0 on success
     * \returns -1 on error
     */ 
   int shMemImName( const std::string & name,     ///< [in] the remote name of a shared memory image.
                    const std::string & localName ///< [in] the local name of a shared memory image.
   );
   
   /// Get the name of the shared memory image.
   /**
     * \returns the name of the shared memory image, the current value of m_shMemImName.
     */ 
   std::string shMemImName(size_t imno);
   
   
   /// Get the name of the local shared memory image.
   /**
     * \returns the name of the local shared memory image, the current value of m_localShMemImName.
     */ 
   std::string localShMemImName(size_t imno);
   
private:
   ///Thread starter, called by imageThreadStart on thread construction.  Calls imageThreadExec.
   static void internal_imageThreadStart( s_imageThread* mit /**< [in] a pointer to an s_imageThread structure */);

public:
   /// Start the image thread.
   int imageThreadStart( size_t thno /**< [in] the thread to start */ );

   /// Execute the image thread.
   void imageThreadExec( const std::string & imageName,     ///< [in] the name of the remote image stream to subscribe to
                         const std::string & localImageName ///< [in] the local name for the image stream
                       );

   /// Flag to control execution.  When true all threads will exit.
   static bool m_timeToDie;
   
   /// Signal the image thread to kill it.
   int imageThreadKill( size_t thno /**< [in] the thread to kill */ );
   
   /** \name Error Handling
     * Errors are reported using a virtual function, so that custom handling can be implemented.
     *
     * @{
     */
   virtual void reportError( const std::string & msg,  ///< [in] the error message
                             const std::string & file, ///< [in] the file which is the source of the error (use __FILE__)
                             int line                  ///< [in] the line number which is the source of the error (use __FILE__)
                           );
   ///@}
};

bool milkzmqClient::m_timeToDie = false;

inline
milkzmqClient::milkzmqClient()
{
   //m_ZMQ_context = new zmq::context_t(1);
   m_ZMQ_context = zmq_ctx_new();
}

inline
milkzmqClient::~milkzmqClient()
{
   m_timeToDie = true;
   
   for(size_t n = 0; n < m_imageThreads.size(); ++n)
   {
      if(m_imageThreads[n].m_thread != nullptr)
      {
         if(m_imageThreads[n].m_thread->joinable()) m_imageThreads[n].m_thread->join();
         delete m_imageThreads[n].m_thread;
         m_imageThreads[n].m_thread = nullptr;
      }
   }
   
   //if(m_ZMQ_context) delete m_ZMQ_context;
   std::cerr << "x\n";
   if(m_ZMQ_context) zmq_ctx_destroy(m_ZMQ_context);
   std::cerr << "y\n";
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
   return shMemImName(name, name);
}

inline
int milkzmqClient::shMemImName( const std::string & name,
                                const std::string & localName
                              )
{
   s_imageThread nt;
   
   nt.m_mzc = this;
   nt.m_imageName = name;
   nt.m_localImageName = localName;
   
   m_imageThreads.push_back(nt);
   
   return 0;
}

inline
std::string milkzmqClient::shMemImName(size_t imno)
{
   if(imno >= m_imageThreads.size()) return "";
   
   return m_imageThreads[imno].m_imageName;
}

inline
std::string milkzmqClient::localShMemImName(size_t imno)
{
   if(imno >= m_imageThreads.size()) return "";
   
   return m_imageThreads[imno].m_localImageName;
}

inline
void milkzmqClient::internal_imageThreadStart( s_imageThread* mit  )
{
   mit->m_mzc->imageThreadExec(mit->m_imageName, mit->m_localImageName);
}

inline
int milkzmqClient::imageThreadStart(size_t thno)
{
   try
   {
      *m_imageThreads[thno].m_thread = std::thread( internal_imageThreadStart, &m_imageThreads[thno]);      
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
   
   if(!m_imageThreads[thno].m_thread->joinable())
   {
      reportError("image thread did not start" , __FILE__, __LINE__);
      return -1;      
   }
   
   return 0;
}

inline
void milkzmqClient::imageThreadExec( const std::string & imageName,
                                     const std::string & localImageName 
                                   )
{   
   //size_t type_size = 0; ///< The size, in bytes, of the image data type

   std::string srvstr = "udp://" + m_address + ":" + std::to_string(m_imagePort);
   
   std::cout << "milkzmqClient: Beginning receive at " << srvstr << " for " << imageName << "\n";
   
   //zmq::socket_t subscriber (*m_ZMQ_context, ZMQ_DISH);
   
   void *dish = zmq_socket(m_ZMQ_context, ZMQ_DISH);
   
   if (zmq_bind(dish, srvstr.c_str()) != 0) 
   {
      printf("Failed to bind listen socket.");
      return;
   }

   std::cerr << "connected\n";

   if (zmq_join(dish, imageName.c_str()) != 0) 
   {
      printf("Could not subscribe to %s.", imageName.c_str());
      return;
   }
    
   std::cerr << "joined\n";
   
   std::string shMemImName;
   if(localImageName == "") shMemImName = imageName;
   else shMemImName = localImageName;
   
   uint8_t new_atype, atype=0;
   uint64_t new_nx, nx =0;
   uint64_t new_ny, ny =0;
   
   /* Initialize ImageStreamIO
    */
   
   IMAGE image;
   bool opened = false;
    
   uint32_t imsize[3];
    
   int curr_image;
   
   uint64_t last_cnt0 = (uint64_t) (-1);
   
   std::vector<uint32_t> msgNumsReceived;
   std::vector<uint8_t> receivedData;
   
   bool finished = true;
   while(!m_timeToDie)
   {
      //zmq::message_t msg;
      
      int bytesReceived;
      zmq_msg_t receiveMessage;

      zmq_msg_init(&receiveMessage);
      bytesReceived = zmq_msg_recv(&receiveMessage, dish, 0);
    
      //std::cerr << "received: " << bytesReceived << "\n";
      
      std::string group = zmq_msg_group(&receiveMessage);
      
      //std::cerr << group << ": ";
      char *msgBuff = (char *) zmq_msg_data(&receiveMessage);
      size_t rSz = zmq_msg_size(&receiveMessage);
               
      uint64_t cnt0 = *((uint64_t *) (msgBuff + cnt0Offset));
      uint32_t msgNum = *((uint32_t *) (msgBuff + msgNumOffset));
      uint32_t msgNumTot = *((uint32_t *) (msgBuff + msgNumTotalOffset));
      
      //std::cerr << cnt0 << " " << msgNum << "/" << msgNumTot << "\n";
      
      if(cnt0 != last_cnt0)
      {
         if(!finished) std::cerr << "missed frame " << last_cnt0 << "\n";
         
         msgNumsReceived.resize(msgNumTot);
         for(int n=0;n<msgNumsReceived.size(); ++n) msgNumsReceived[n] = 0;
         
         last_cnt0 = cnt0;
         
         finished = false;
      }
      
      msgNumsReceived[msgNum] = 1;
      

      if( receivedData.size() < msgNum*1024 + rSz - payloadOffset) 
      {
         receivedData.resize( msgNum*1024 + rSz - payloadOffset);
      }
      
      for(size_t i=0;i<rSz-payloadOffset; ++i) receivedData[ msgNum*1024 + i] = msgBuff[payloadOffset + i];
      
      zmq_msg_close(&receiveMessage);

      int numReceived = 0;
      for(size_t n=0;n<msgNumsReceived.size(); ++n) numReceived += msgNumsReceived[n];
      
      if(numReceived == msgNumTot)
      {
         finished = true;
      
         char * raw_image= (char *) receivedData.data();
      
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
      
         image.md[0].cnt0 = cnt0;//*( (uint64_t *) (raw_image + cnt0Offset));
         image.md[0].atime.tv_sec = *( (uint64_t *) (raw_image + tv_secOffset));
         image.md[0].atime.tv_nsec = *( (uint64_t *) (raw_image + tv_nsecOffset));
         //std::cerr << image.md[0].cnt0 << " " << image.md[0].atime.tv_sec << " " << image.md[0].atime.tv_nsec << "\n";
         
         memcpy(image.array.SI8 + curr_image*nx*ny*type_size, raw_image + imageOffset, nx*ny*type_size);
         
           
      
         image.md[0].cnt1=0;
         image.md[0].write=0;
         ImageStreamIO_sempost(&image,-1);
         
      }
   }

   if(opened) ImageStreamIO_closeIm(&image);
   
   
} // milkzmqClient::imageThreadExec()

inline
int milkzmqClient::imageThreadKill(size_t thno)
{
   pthread_kill(m_imageThreads[thno].m_thread->native_handle(), SIGTERM);
   return 0;
}
      
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
