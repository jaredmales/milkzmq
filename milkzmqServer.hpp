/** \file milkzmqServer.hpp
  * \brief Class implementing a ZeroMQ ImageStreamIO server
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

#ifndef milkzmqServer_hpp
#define milkzmqServer_hpp

#include <signal.h>
#include <fcntl.h>  // for open

#include <unordered_map>
#include <mutex>

#define ZMQ_BUILD_DRAFT_API
#define ZMQ_CPP11
#include <zmq.hpp>

#include "milkzmqUtils.hpp"

namespace milkzmq 
{



///\todo need to handle errors/exceptions from zmq API
class milkzmqServer
{
protected:
   
   /** \name Configurable Parameters 
     *
     *@{
     */
   
   std::string m_argv0 {"milkzmqServer"}; ///< The invoked name, used for error messages.
   
   int m_imagePort{5556}; ///< The port number to use for the image server.
   
   std::string m_shMemImName; ///< The name of the ImageStreamIO shared memory image
   
   int m_usecSleep {100}; ///< The number of microseconds to sleep on each loop.  Default 100.
   
   float m_fpsTgt{10}; ///< The max frames per second (f.p.s.) to transmit data.
   
   float m_fpsGain{0.1}; ///< Integrator gain on the fps trigger delta.
   
   int m_xrifDifferenceMethod {XRIF_DIFFERENCE_NONE}; ///< The difference method to use.
   
   int m_xrifReorderMethod {XRIF_REORDER_NONE};   ///< The method to use for bit reordering.
   
   int m_xrifCompressMethod {XRIF_COMPRESS_NONE}; ///< The compression method used.
   
   ///@}
   
   /** \name Internal State 
     *
     *@{
     */

   zmq::context_t * m_ZMQ_context {nullptr}; ///< The ZeroMQ context, allocated on construction.

   zmq::socket_t * m_server {nullptr};  ///< The ZeroMQ server, allocated when the server thread starts up.
   
   std::thread m_serverThread;
   
   typedef uint32_t routing_id_t;
   
   typedef std::unordered_map< std::string, bool> imageReceivedFlagMap_t;
   
   std::unordered_map<routing_id_t, imageReceivedFlagMap_t> m_requestorMap;
   
   ///Mutex for locking map operations (allows asynchronous deletes).
   std::mutex m_mapMutex;
   
   ///Structure to manage the image threads, including startup.
   struct s_imageThread
   {
      std::thread * m_thread {nullptr}; ///< Thread for publishing image slice updates.  A pointer to allow copying, but must be deleted in d'tor of parent.
      milkzmqServer * m_mzs;            ///< a pointer to a milkzmqServer instance (normally this)
      std::string m_imageName;          ///< the name of the image to serve from this thread
      
      ///C'tor to create the thread object
      s_imageThread()
      {
         m_thread = new std::thread;
      }      
   };

   std::vector<s_imageThread> m_imageThreads; ///< The image threads, one per shared memory streamm being served.
   
   
   ///@}
   
public:
   
   /// Default c'tor
   milkzmqServer();
   
   /// Destructor
   ~milkzmqServer();
   
   /// Set the invoked name of the application.
   /** This sets the value of m_argv0, used for error reporting.
     * 
     * \returns 0 on success
     * \returns -1 on error
     */
   int argv0( const std::string & av0 );
   
   /// Get the invoked name
   /**
     * \returns the invoked name, the current value of m_argv0
     */
   std::string argv0();
   
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
   
   /// Add the name of the ImageStreamIO shared memory image to the list
   /** This is just the root.  E.g. for a complete path of '/tmp/image00.im.shm' the argument should be "image00".
     * This extends the m_imageThreads vector.
     * 
     * \returns 0 on success
     * \returns -1 on error
     */ 
   int shMemImName( const std::string & name /**< [in] the  new name of the shared memory image.*/);
   
   /// Get the number of shared memory images being monitored.
   /**
     * \returns the name of the result of m_imageThreads.size().
     */
   size_t numImages();
   
   /// Get the name of the shared memory image.
   /**
     * \returns the name of the shared memory image, the current value of m_shMemImName.
     */ 
   std::string shMemImName(size_t n);
      
   /// Set the time to sleep between semaphore checks.
   /** Use this to tune the responsiveness vs. cpu use.
     * 
     * This sets the value of m_usecSleep.
     * 
     * \returns 0 on success
     * \returns -1 on error
     */
   int usecSleep( const int & usec /**< [in] the new sleep time in microseconds*/);
   
   /// Get the time to sleep between semaphore checks
   /** 
     * \returns the curren value of m_usecSleep.
     */ 
   int usecSleep();
   
   /// Set the target F.P.S. served.
   /**
     * \returns 0 on success
     * \returns -1 on error
     */
   int fpsTgt(const float & fps /**< [in] the new fps target*/);
   
   /// Get the target maximum F.P.S.
   /**
     * \returns the target fps, the current value of m_fpsTgt
     */ 
   float fpsTgt();
   
   /// Set the gain of the F.P.S. error loop.
   /** Uses an integrator controller to keep the image serving rate as close to m_fpsTgt as possible.
     * 
     * \returns 0 on success
     * \returns -1 on error
     */
   int fpsGain(const float & gain /**< [in] the new fps gain value*/);
   
   /// Get the gain of the F.P.S. error loop.
   /**
     * \returns the gain of the fps integrator, m_fpsGain.
     */ 
   float fpsGain();
   
   /// Disable compression.
   /** Turns off compression.
     */
   void noCompression();
   
   /// Enable default compression.
   /** Default compression is XRIF_DIFFERENCE_PIXEL, XRIF_REORDER_BYTEPACK_RENIBBLE, XRIF_COMPRESS_LZ4.
     */
   void defaultCompression();

   /// Set the XRIF encoding difference method.
   /** 
     * \returns 0 on success
     * \returns -1 on error
     */
   int xrifDifferenceMethod(const int & xdm /**< [in] The new differencing method */);
   
   /// Get the XRIF encoding difference method.
   /**
     * \returns the current value of m_xrifDifferenceMethod.
     */ 
   int xrifDifferenceMethod();
   
   /// Set the XRIF encoding reordering method.
   /** 
     * \returns 0 on success
     * \returns -1 on error
     */
   int xrifReorderMethod(const int & xrm /**< [in] The new reordering method */);
   
   /// Get the XRIF encoding reordering method.
   /**
     * \returns the current value of m_xrifReorderMethod.
     */ 
   int xrifReorderMethod();
   
   /// Set the XRIF encoding compression method.
   /** 
     * \returns 0 on success
     * \returns -1 on error
     */
   int xrifCompressMethod(const int & xcm /**< [in] The new compression method */);
   
   /// Get the XRIF encoding compression method.
   /**
     * \returns the current value of m_xrifCompressMethod.
     */ 
   int xrifCompressMethod();
   
private:
   
   ///Server thread starter, called by serverThreadStart on thread construction.  Calls serverThreadExec.
   static void internal_serverThreadStart( milkzmqServer * mzs /**< [in] a pointer to a milkzmqServer instance, usually this */);

public:
   /// Start the server thread.
   int serverThreadStart();

   /// Execute the image thread.
   void serverThreadExec();
   
   /// Signal the server thread to kill it.
   int serverThreadKill( );
   
private:
   
   ///Image thread starter, called by imageThreadStart on thread construction.  Calls imageThreadExec.
   static void internal_imageThreadStart( s_imageThread* mit /**< [in] a pointer to an s_imageThread structure */);

public:
   /// Start the image thread.
   int imageThreadStart(size_t thno /**< [in] the thread to start */);

   /// Execute the image thread.
   void imageThreadExec(const std::string & imageName /**< [in] the name of the image stream to monitor and publish*/ );

   /// Flag to control execution.  When true all threads will exit.
   static bool m_timeToDie;
   
   /// Signal the image thread to kill it.
   int imageThreadKill( size_t thno /**< [in] the thread to kill */ );
   
   /// Flag to indicate a restart of the image thread loop is needed.
   /** This is intended to be set after a SIGSEGV or SIGBUS is recieved, which tends
     * to occur if the source of the images exits, causing the
     * shared mem stream to go wonky.
     */ 
   static bool m_restart;
   
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

bool milkzmqServer::m_timeToDie = false;
bool milkzmqServer::m_restart = false;

inline
milkzmqServer::milkzmqServer()
{
   milkzmq_argv0 = m_argv0; //set the global
   ImageStreamIO_set_printError(milkzmq_printError);
      
   m_ZMQ_context = new zmq::context_t;
}

inline
milkzmqServer::~milkzmqServer()
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
   
   if(m_server) m_server->close();
   
   if(m_ZMQ_context) delete m_ZMQ_context;
   
   pthread_kill(m_serverThread.native_handle(), SIGINT);
   if(m_serverThread.joinable()) m_serverThread.join();
   
  
   
}

inline 
int milkzmqServer::argv0( const std::string & av0 )
{
   m_argv0 = av0;
   milkzmq_argv0 = m_argv0; //set the global
   
   return 0;
}

inline 
std::string milkzmqServer::argv0()
{
   return m_argv0;
}

inline
int milkzmqServer::imagePort( const int & imagePort )
{
   m_imagePort = imagePort;
   
   return 0;
}

inline
int milkzmqServer::imagePort()
{
   return m_imagePort;
}


inline
int milkzmqServer::shMemImName( const std::string & name )
{
   s_imageThread nt;
   
   nt.m_mzs = this;
   nt.m_imageName = name;
   
   m_imageThreads.push_back(nt);
   
   return 0;
}

inline
size_t milkzmqServer::numImages()
{
   return m_imageThreads.size();
}

inline
std::string milkzmqServer::shMemImName(size_t n)
{
   return m_imageThreads[n].m_imageName;
}

inline
int milkzmqServer::usecSleep( const int & usec )
{
   m_usecSleep = usec;
   return 0;
}

inline
int milkzmqServer::usecSleep()
{
   return m_usecSleep;
}

inline
int milkzmqServer::fpsTgt(const float & fps )
{
   m_fpsTgt = fps;
   return 0;
}

inline
float milkzmqServer::fpsTgt()
{
   return m_fpsTgt;
}
 
inline
int milkzmqServer::fpsGain(const float & gain )
{
   m_fpsGain = gain;
   return 0;
}

inline
float milkzmqServer::fpsGain()
{
   return m_fpsGain;
}

inline
void milkzmqServer::noCompression()
{
   xrifDifferenceMethod(XRIF_DIFFERENCE_NONE);
   xrifReorderMethod(XRIF_REORDER_NONE);
   xrifCompressMethod(XRIF_COMPRESS_NONE);
}

inline
void milkzmqServer::defaultCompression()
{
   xrifDifferenceMethod(XRIF_DIFFERENCE_PIXEL);
   xrifReorderMethod(XRIF_REORDER_BYTEPACK_RENIBBLE);
   xrifCompressMethod(XRIF_COMPRESS_LZ4);
}

inline
int milkzmqServer::xrifDifferenceMethod(const int & xdm)
{
   m_xrifDifferenceMethod = xdm;
   return 0;
}

inline
int milkzmqServer::xrifDifferenceMethod()
{
   return m_xrifDifferenceMethod;
}

inline
int milkzmqServer::xrifReorderMethod(const int & xrm)
{
   m_xrifReorderMethod = xrm;
   return 0;
}

inline
int milkzmqServer::xrifReorderMethod()
{
   return m_xrifReorderMethod;
}

inline
int milkzmqServer::xrifCompressMethod(const int & xcm)
{
   m_xrifCompressMethod = xcm;
   return 0;
}

inline
int milkzmqServer::xrifCompressMethod()
{
   return m_xrifCompressMethod;
}
   
inline
void milkzmqServer::internal_serverThreadStart( milkzmqServer * mzs )
{
   mzs->serverThreadExec();
}

inline
int milkzmqServer::serverThreadStart()
{
   try
   {
      m_serverThread = std::thread( internal_serverThreadStart, this);
   }
   catch( const std::exception & e )
   {
      reportError(std::string("exception in server thread startup: ") +e.what(), __FILE__, __LINE__);
      return -1;
   }
   catch( ... )
   {
      reportError("unknown exception in server thread startup", __FILE__, __LINE__);
      return -1;
   }
   
   if(!m_serverThread.joinable())
   {
      reportError("server thread did not start", __FILE__, __LINE__);
      return -1;
   }
   
   return 0;
}

inline
void milkzmqServer::serverThreadExec()
{   
   std::string srvstr = "tcp://*:" + std::to_string(m_imagePort);
   
   reportInfo("Beginning service at " + srvstr);
   
   //Should be nullptr, but in case this gets called twice.
   if(m_server)
   {
      m_server->close();
      delete m_server;
      m_server = nullptr;
   }
      
   m_server = new zmq::socket_t(*m_ZMQ_context, ZMQ_SERVER);
     
   m_server->bind(srvstr);
   
   char reqShmim[1024];
  
   std::cerr << "here we go\n";
 
   while(!m_timeToDie) //loop on timeToDie in case this gets interrupted by SIGSEGV/SIGBUS
   {
      zmq::message_t request;

      try
      {
         //Wait for next request from a client
         #if(CPPZMQ_VERSION >= ZMQ_MAKE_VERSION(4, 3, 1))
         m_server->recv (request);
         #else
         m_server->recv(&request);
         #endif
      }
      catch(...)
      {
         if(m_timeToDie) break; //If an exception is thrown we check for timeToDie
         throw;
      }
      
      uint32_t routing_id = request.routing_id();
      
      size_t sz = sizeof(reqShmim);
      if(request.size() +1 < sz) sz = request.size()+1;
      snprintf(reqShmim, sz, "%s", (char*)request.data());
      
      //Scope for map mutex
      {
         std::lock_guard<std::mutex> guard(m_mapMutex);
      
         //All we do is set the received flag to true for this client and shmim, which tells the image thread to go ahead and send next time.
         m_requestorMap[routing_id][reqShmim] = true;
      }
   }
            
   
} // milkzmqServer::serverThreadExec()

inline
int milkzmqServer::serverThreadKill()
{
   pthread_kill(m_serverThread.native_handle(), SIGQUIT);
   return 0;
}

inline
void milkzmqServer::internal_imageThreadStart( s_imageThread * mit )
{
   mit->m_mzs->imageThreadExec(mit->m_imageName);
}

inline
int milkzmqServer::imageThreadStart(size_t thno)
{
   try
   {
      *m_imageThreads[thno].m_thread = std::thread( internal_imageThreadStart, &m_imageThreads[thno]);
   }
   catch( const std::exception & e )
   {
      reportError(std::string("exception in image thread startup: ") +e.what(), __FILE__, __LINE__);
      return -1;
   }
   catch( ... )
   {
      reportError("unknown exception in image thread startup", __FILE__, __LINE__);
      return -1;
   }
   
   if(!m_imageThreads[thno].m_thread->joinable())
   {
      reportError("image thread did not start", __FILE__, __LINE__);
      return -1;
   }
   
   return 0;
}

inline
int milkzmqServer::imageThreadKill(size_t thno)
{
   pthread_kill(m_imageThreads[thno].m_thread->native_handle(), SIGQUIT);
   return 0;
}



inline
void milkzmqServer::imageThreadExec(const std::string & imageName)
{   
   IMAGE image;

   size_t type_size = 0; ///< The size, in bytes, of the image data type

   bool opened = false;
   
   uint8_t * msg = nullptr;
   
   
   while(m_server == nullptr)
   {
      milkzmq::sleep(1);
   }
   
   xrif_error_t xe;
   xrif_t xrif = nullptr;
   xe = xrif_new(&xrif);
   
                              
   while(!m_timeToDie)
   {
      opened = false;
      m_restart = false; //Set this up front, since we're about to restart.
      
      int printed = 0;
      while(!opened && !m_timeToDie && !m_restart)
      {
         //b/c ImageStreamIO prints every single time, and latest version don't support stopping it yet, and that isn't thread-safe-able anyway
         //we do our own checks.  This is the same code in ImageStreamIO_openIm...
         int SM_fd;
         char SM_fname[200];
         ImageStreamIO_filename(SM_fname, sizeof(SM_fname), imageName.c_str());
         SM_fd = open(SM_fname, O_RDWR);
         if(SM_fd == -1)
         {
            if(!printed) reportWarning("ImageStream " + imageName + " not found (yet).  Retrying . . .");
            printed = 1;
            milkzmq::sleep(1); //be patient
            continue;
         }
         
         //Found and opened,  close it and then use ImageStreamIO
         printed = 0;
         close(SM_fd);
         
         if( ImageStreamIO_openIm(&image, imageName.c_str()) == 0)
         {
            if(image.md[0].sem <= 0) 
            {
               ImageStreamIO_closeIm(&image);
               milkzmq::sleep(1); //We just need to wait for the server process to finish startup.
            }
            else
            {
               type_size = ImageStreamIO_typesize(image.md[0].datatype);
               opened = true;
            }
         }
         else
         {
            milkzmq::sleep(1); //be patient
         }
      }
      if(m_timeToDie || !opened) return;
    
      reportNotice("Connected to ImageStream " + imageName);
      
      int curr_image;
      uint8_t atype;
      uint32_t snx, sny, snz;
      uint8_t last_atype = image.md[0].datatype;
      uint32_t last_snx = image.md[0].size[0];
      uint32_t last_sny = image.md[0].size[1];
      uint32_t last_snz = image.md[0].size[2];

      //---- Set up xrif handle      
      xe = xrif_set_size(xrif, last_snx, last_sny, 1, 1, last_atype);
      xe = xrif_configure(xrif, m_xrifDifferenceMethod, m_xrifReorderMethod, m_xrifCompressMethod);
      
      //---- Allocate the message
      if(msg != nullptr) 
      {
         //problem: if msg is not nullptr, then it means that it was allocated and 
         //most likely handed over to zmq for sending.  We can't afford to free it 
         //until zmq actually decides it's done with it.
         //for now: we sleep.
         sleep(2);
         free(msg);
      }
      size_t msgSz = headerSize + xrif_min_raw_size(xrif); //This is maximum message size.
      msg = (uint8_t *) malloc(msgSz);
      
      //---- Allocate XRIF
      xe = xrif_set_size(xrif, last_snx, last_sny, 1, 1, last_atype);
      xe = xrif_set_raw(xrif, msg + headerSize, xrif_min_raw_size(xrif));
      xe = xrif_allocate_reordered(xrif);
      
      double lastCheck = get_curr_time();
      double lastSend = get_curr_time();
      double delta = 0;
      
      uint64_t lastCnt0 = -1; //Force treating first image as new image
      
      std::vector<routing_id_t> rids;
      
      while(!m_timeToDie && !m_restart)
      {
         uint64_t cnt0 = image.md[0].cnt0;
         if(cnt0 != lastCnt0)
         {
            //-------- Do a wait for max fps here.
            double currtime = get_curr_time();
            if( currtime - lastCheck < 1.0/m_fpsTgt-delta) 
            {
               milkzmq::microsleep(m_usecSleep);
               continue;
            }
            lastCheck = currtime;

            //-------- Check to see if anyone is subscribing to this stream...
            rids.clear();
            
            //We lock the mutex during lookup, but unlock so that it isn't blocked during the send call.
            //Scope for map mutex
            {
               std::lock_guard<std::mutex> guard(m_mapMutex);

               std::unordered_map<routing_id_t, imageReceivedFlagMap_t>::iterator it = m_requestorMap.begin();
            
               while(it != m_requestorMap.end())
               {
                  if( it->second.count(imageName) > 0 )
                  {
                     if(it->second[imageName] == true)
                     {
                        rids.push_back(it->first);
                     }
                  }
                  ++it;
               }
            }
            if(rids.size() == 0) continue; //No subscribers
            
            if(m_timeToDie || m_restart) break; //Check for exit signals

            //-------- Get size and look for changes
            atype = image.md[0].datatype;
            snx = image.md[0].size[0];
            sny = image.md[0].size[1];
            snz = image.md[0].size[2];
         
            if( atype!= last_atype || snx != last_snx || sny != last_sny || snz != last_snz )
            {
               break; //exit the nearest while loop and get the new image setup.
            }
            
            if(image.md[0].size[2] > 0) ///\todo change to naxis?
            {
               curr_image = image.md[0].cnt1;
               if(curr_image < 0) curr_image = image.md[0].size[2] - 1;
            }
            else curr_image = 0;

            cnt0 = image.md[0].cnt0;
            
            
            //XRIF Encoding...
            //Because of xrif->compress_on_raw == true, xrif->raw_buffer = msg + headerSize, this results in the encoded message being written to the message buffer.
            memcpy(xrif->raw_buffer, image.array.SI8 + curr_image*snx*sny*type_size, snx*sny*type_size);
            xe = xrif_encode(xrif);
   
            //std::cerr << imageName << " XRIF: " << xrif->compression_ratio*100. << "%\n";

            //----Now construct header
            memset(msg, 0, headerSize);
            snprintf((char *) msg, nameSize, "%s", imageName.c_str());
            *((uint8_t *) (msg + typeOffset)) = atype;
            *((uint32_t *) (msg + size0Offset)) = snx;
            *((uint32_t *) (msg + size1Offset)) = sny;
            *((uint64_t *) (msg + cnt0Offset)) = image.md[0].cnt0;
            *((uint64_t *) (msg + tv_secOffset)) = image.md[0].atime.tv_sec;
            *((uint64_t *) (msg + tv_nsecOffset)) = image.md[0].atime.tv_nsec;
            *((int16_t *) (msg + xrifDifferenceOffset)) = xrif->difference_method; 
            *((int16_t *) (msg + xrifReorderOffset))    = xrif->reorder_method;
            *((int16_t *) (msg + xrifCompressOffset))   = xrif->compress_method;
            *((uint32_t *) (msg + xrifSizeOffset))  = xrif->compressed_size;
            
            
            //memcpy(msg + imageOffset, image.array.SI8 + curr_image*snx*sny*type_size, snx*sny*type_size);
            //memcpy(msg+imageOffset, xrif->raw_buffer, xrif->compressed_size);
            
            if(m_timeToDie || m_restart) break; //Check for exit signals
            
            if( rids.size() > 0 )
            {
               for(size_t rid = 0; rid < rids.size(); ++rid)
               {
                  zmq::message_t frame( msg, headerSize + xrif->compressed_size, nullptr, nullptr);//this version will not copy the data.
                  frame.set_routing_id(rids[rid]);
                  try
                  {
                     #if(CPPZMQ_VERSION >= ZMQ_MAKE_VERSION(4, 3, 1))
                     m_server->send(frame, zmq::send_flags::dontwait);
                     #else
                     m_server->send(frame, ZMQ_DONTWAIT);
                     #endif
                  
                     std::lock_guard<std::mutex> guard(m_mapMutex);
                     m_requestorMap[rids[rid]][imageName] = false;
                  }
                  catch(...)
                  {
                     //Assume this means the client is no longer connected
                     std::lock_guard<std::mutex> guard(m_mapMutex);
                     m_requestorMap.erase(rids[rid]);
                  }
               }
            }            
            
            
            double ct = get_curr_time();
            
            //An integrator controller to keep frame rate on target
            delta += m_fpsGain * (ct-lastSend - 1.0/m_fpsTgt);
            lastSend = ct;
            lastCnt0 = cnt0;
            
         }
         else
         {
            if(image.md[0].sem <= 0) break; //Indicates that the server has cleaned up.
            
            milkzmq::microsleep(m_usecSleep);
            
            //If delay is long, we reset the loop b/c we aren't doing any good anyway, and this prevents over shoot on a reconnect
            if(get_curr_time() - lastSend > 2*1.0/m_fpsTgt)
            {
               delta = 0;
               lastSend = get_curr_time() - 2*1.0/m_fpsTgt;
            }
         }
      }

      if(opened) 
      {
         ImageStreamIO_closeIm(&image);
         opened = false;
      } 
   }
   
   //-------- Send a 0 message to tell the other side to hangup...
   std::vector<routing_id_t> rids;
   //Scope for map mutex
   {
      std::lock_guard<std::mutex> guard(m_mapMutex);

      std::unordered_map<routing_id_t, imageReceivedFlagMap_t>::iterator it = m_requestorMap.begin();
            
      while(it != m_requestorMap.end())
      {
         if( it->second.count(imageName) > 0 )
         {
            if(it->second[imageName] == true)
            {
               rids.push_back(it->first);
            }
         }
         ++it;
      }
   }
   if( rids.size() > 0 )
   {
      char zero = '\0';
      
      for(size_t rid = 0; rid < rids.size(); ++rid)
      {
         zmq::message_t frame( &zero, sizeof(char), nullptr, nullptr);
         frame.set_routing_id(rids[rid]);
         try
         {
            #if(CPPZMQ_VERSION >= ZMQ_MAKE_VERSION(4, 3, 1))
            m_server->send(frame, zmq::send_flags::dontwait);
            #else
            m_server->send(frame, ZMQ_DONTWAIT);
            #endif
         
            std::lock_guard<std::mutex> guard(m_mapMutex);
            m_requestorMap[rids[rid]][imageName] = false;
         }
         catch(...)
         {
            //Assume this means the client is no longer connected
            std::lock_guard<std::mutex> guard(m_mapMutex);
            m_requestorMap.erase(rids[rid]);
         }
      }
   }            
   
   
   
   //One more check
   if(opened) ImageStreamIO_closeIm(&image);
   if(msg) free(msg);
   if(xrif != nullptr) xrif_delete(xrif);
   
} // milkzmqServer::imageThreadExec()

inline 
void milkzmqServer::reportInfo( const std::string & msg )
{
   milkzmq::reportInfo(m_argv0, msg);
}

inline 
void milkzmqServer::reportNotice( const std::string & msg )
{
   milkzmq::reportNotice(m_argv0, msg);
}

inline 
void milkzmqServer::reportWarning( const std::string & msg )
{
   milkzmq::reportWarning(m_argv0, msg);
}

inline 
void milkzmqServer::reportError( const std::string & msg,
                                 const std::string & file,
                                 int line
                               )
{
   milkzmq::reportError(m_argv0, msg, file, line);
}


} //namespace milkzmq 

#endif //milkzmqServer_hpp
