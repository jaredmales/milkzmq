/** \file milkzmqClient.hpp
  * \brief Class implementing a ZeroMQ ImageStreamIO client
  * \author Jared R. Males (jaredmales@gmail.com)
  *
  * History:
  * - 2018 created by JRM
  */

//***********************************************************************//
// Copyright 2018-2021 Jared R. Males (jaredmales@gmail.com)
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

#define ZMQ_BUILD_DRAFT_API
#define ZMQ_CPP11
#include <zmq.hpp>

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
   
  /** \name Status and Error Handling
     * Status updates, warnings, and errors are reported using virtual functions, so that custom handling can be implemented.
     *
     * @{
     */
   
   /// Report status (with LOG_INFO level of priority) to the user using stderr.
   virtual void reportInfo( const std::string & msg /**< [in] the status message */);
   
   /// Report status (with LOG_NOTICE level of priority) to the user using stderr.
   virtual void reportNotice( const std::string & msg /**< [in] the status message */);
   
   /// Report a warning to the user using stderr.
   virtual void reportWarning( const std::string & msg /**< [in] the warning message */);
   
   /// Report an error to the user using stderr.
   virtual void reportError( const std::string & msg,  ///< [in] the error message 
                             const std::string & file, ///< [in] the name of the file where the error occurred
                             int line                  ///< [in] the line number of the error
                           );

   ///@}
};

bool milkzmqClient::m_timeToDie = false;

inline
milkzmqClient::milkzmqClient()
{
   milkzmq_argv0 = m_argv0; //set the global
   ImageStreamIO_set_printError(milkzmq_printError);
   
   m_ZMQ_context = new zmq::context_t(1);
}

inline
milkzmqClient::~milkzmqClient()
{
   m_timeToDie = true;

   if(m_ZMQ_context) delete m_ZMQ_context;
   
   for(size_t n = 0; n < m_imageThreads.size(); ++n)
   {
      if(m_imageThreads[n].m_thread != nullptr)
      {
         if(m_imageThreads[n].m_thread->joinable()) m_imageThreads[n].m_thread->join();
         delete m_imageThreads[n].m_thread;
         m_imageThreads[n].m_thread = nullptr;
      }
   }
   

}

inline 
int milkzmqClient::argv0( const std::string & av0 )
{
   m_argv0 = av0;
   milkzmq_argv0 = m_argv0; //set the global
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
   std::string srvstr = "tcp://" + m_address + ":" + std::to_string(m_imagePort);
   
   reportInfo("Beginning receive at " + srvstr + " for " + imageName);
   
   
   std::string shMemImName;
   if(localImageName == "") shMemImName = imageName;
   else 
   {
      shMemImName = localImageName;
      reportInfo("Writing " + imageName + " to " + shMemImName);
   }
      
   uint8_t new_atype, atype=0;
   uint64_t new_nx, nx =0;
   uint64_t new_ny, ny =0;
   
   /* Initialize xrif
    */
   xrif_error_t xe;
   xrif_t xrif;
   xe = xrif_new(&xrif);
   
   /* Initialize ImageStreamIO
    */
     
   IMAGE image;
   bool opened = false;
       
   uint32_t imsize[3];
       
   int curr_image;
      
   //Outer loop, which will periodically refresh the subscription if needed.
   while(!m_timeToDie)
   {
      zmq::socket_t subscriber (*m_ZMQ_context, ZMQ_CLIENT);
   
      #if(CPPZMQ_VERSION >= ZMQ_MAKE_VERSION(4, 3, 1))
      subscriber.set(zmq::sockopt::linger, 1000);
      subscriber.set(zmq::sockopt::linger, 0);
      #else
      subscriber.setsockopt(ZMQ_RCVTIMEO, 1000);
      subscriber.setsockopt(ZMQ_LINGER, 0);
      #endif
      
      subscriber.connect(srvstr);
   
      zmq::message_t request(imageName.data(), imageName.size());
      
      #if(CPPZMQ_VERSION >= ZMQ_MAKE_VERSION(4, 3, 1))
      subscriber.send(request, zmq::send_flags::none);
      #else
      subscriber.send(request);
      #endif
      bool reconnect = false;
      
      
      bool first = true;
      bool connected = false;
      
      #ifdef MZMQ_FPS_MONITORING
      int Nrecvd = 100;
      double t0 = 0, t1;
      #endif
      
      while(!m_timeToDie && !reconnect) //Inner loop waits for each new image and processes it as it comes in.
      {
         zmq::message_t msg;
           
         #if(CPPZMQ_VERSION >= ZMQ_MAKE_VERSION(4, 7, 0))
         zmq::recv_result_t recvd;
         #elif(CPPZMQ_VERSION >= ZMQ_MAKE_VERSION(4, 3, 1))
         zmq::detail::recv_result_t recvd;
         #else
         size_t recvd; 
         #endif
         
         try
         {
            #if(CPPZMQ_VERSION >= ZMQ_MAKE_VERSION(4, 3, 1))
               recvd = subscriber.recv(msg);
            #else
               recvd = subscriber.recv(&msg); 
            #endif
         }
         catch(...)
         {
            if(m_timeToDie) break; //This will true be if signaled during shutdown            
            //otherwise, this is an error
            throw;
         }
      
         #if(CPPZMQ_VERSION >= ZMQ_MAKE_VERSION(4, 3, 1))
         if(!recvd)
         {
            if(zmq_errno() == EAGAIN) //If we timed out, just re-send the request
            {
               request.rebuild(imageName.data(), imageName.size());
               subscriber.send(request, zmq::send_flags::none);
               continue;
            }
                        
            if(connected) reportNotice("Disconnected from " + imageName);
            connected = false;
            reconnect = true;
            break;
         }
         #else
         if(recvd == 0)
         {
            if(zmq_errno() == EAGAIN) //If we timed out, just re-send the request
            {
               request.rebuild(imageName.data(), imageName.size());
               subscriber.send(request);
               continue;
            }
            
            if(connected) reportNotice("Disconnected from " + imageName);
            connected = false;
            reconnect = true;
            break;
         }
         #endif
         
         if(first)
         {
            reportNotice("Connected to " + imageName);
            connected = true;
            first = false;
         }
         
         if(msg.size() <= headerSize) //If we don't get enough data, we reconnect to the server.
         {
            sleep(1); //Give server time to finish its shutdown.
            reconnect= true;
            continue;
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
            
            ImageStreamIO_createIm(&image, shMemImName.c_str(), 2, imsize, new_atype, 1, 0, 0);
            
            opened = true;
            
            xe = xrif_set_size(xrif, new_nx, new_ny, 1, 1, new_atype);
            xrif_set_difference_method(xrif, *((int16_t *) (raw_image + xrifDifferenceOffset)));
            xrif_set_reorder_method(xrif, *((int16_t *) (raw_image + xrifReorderOffset)));
            xrif_set_compress_method(xrif, *((int16_t *) (raw_image+ xrifCompressOffset)));
            
            xe = xrif_allocate(xrif);
         }
         
         atype = new_atype;
         nx = new_nx;
         ny = new_ny;
         
      
         //This is not a rolling buffer.
         curr_image = 0;
         
         size_t type_size = ImageStreamIO_typesize(image.md[0].datatype);
      
         image.md[0].write=1;
      
         image.md[0].cnt0 = *( (uint64_t *) (raw_image + cnt0Offset));
         image.md[0].writetime.tv_sec = *( (uint64_t *) (raw_image + tv_secOffset));
         image.md[0].writetime.tv_nsec = *( (uint64_t *) (raw_image + tv_nsecOffset));
         
         xrif->compressed_size =  *((uint32_t *) (raw_image + xrifSizeOffset));
         
         memcpy(xrif->raw_buffer, raw_image + imageOffset, xrif->compressed_size);
         xe = xrif_decode(xrif);
         
         memcpy(image.array.SI8 + curr_image*nx*ny*type_size, xrif->raw_buffer, nx*ny*type_size);
         
         image.md[0].cnt1=0;
         image.md[0].write=0;
         ImageStreamIO_sempost(&image,-1);
         
         #ifdef MZMQ_FPS_MONITORING
         if(Nrecvd >= 10)
         {
            Nrecvd = 0;
            t0 = get_curr_time();
         }
         else ++Nrecvd;
         
         if(Nrecvd >= 10)
         {
            t1 = get_curr_time() - t0;
            std::cerr << imageName << " averaging " << Nrecvd/t1 << " FPS received.\n";
         }
         #endif

         //Here is where we can add client-specefic rate control!
         
         request.rebuild(imageName.data(), imageName.size());
         //memcpy( request.data(), imageName.c_str(), imageName.size());
         #if(CPPZMQ_VERSION >= ZMQ_MAKE_VERSION(4, 3, 1))
         subscriber.send(request, zmq::send_flags::dontwait);
         #else
         subscriber.send(request, ZMQ_DONTWAIT);
         #endif
         
      } // inner loop (image processing)
      
      subscriber.close(); //close so that unsent messages are dropped.
      
      //To trigger a full reconnect:
      
      if(opened) ImageStreamIO_closeIm(&image);
      opened = false;
      
      atype=0;
      nx =0;
      ny =0;
    
      first = true;
      connected = false;
      
      #ifdef MZMQ_FPS_MONITORING
      Nrecvd = 100;
      t0 = 0;
      #endif
      
      reportNotice("Disconnected from " + imageName);
         
   }// outer loop (checking stale connections)
   
   if(opened) ImageStreamIO_closeIm(&image);
   xrif_delete(xrif);
   
} // milkzmqClient::imageThreadExec()

inline
int milkzmqClient::imageThreadKill(size_t thno)
{
   pthread_kill(m_imageThreads[thno].m_thread->native_handle(), SIGTERM);
   return 0;
}
      
inline 
void milkzmqClient::reportInfo( const std::string & msg )
{
   milkzmq::reportInfo(m_argv0, msg);
}

inline 
void milkzmqClient::reportNotice( const std::string & msg )
{
   milkzmq::reportNotice(m_argv0, msg);
}

inline 
void milkzmqClient::reportWarning( const std::string & msg )
{
   milkzmq::reportWarning(m_argv0, msg);
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
