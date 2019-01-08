/** \file milkzmqServer.hpp
  * \brief Class implementing a ZeroMQ ImageStreamIO server
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

#ifndef milkzmqServer_hpp
#define milkzmqServer_hpp

#include <zmq.hpp>

#include <ImageStreamIO.h>

#include "milkzmqUtils.hpp"

namespace milkzmq 
{


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
   int m_sempahoreNumber {0}; ///< The number of the ImageStreamIO semaphore to monitor
   int m_usecSleep {10}; ///< The number of microseconds to sleep on each loop.  Default 100.
   
   float m_fpsTgt{20}; ///< The max frames per second (f.p.s.) to transmit data.
   
   float m_fpsGain{0.1}; ///< Integrator gain on the fps trigger delta.

   ///@}
   
   /** \name Internal State 
     *
     *@{
     */
   
   std::thread m_metaThread;  ///< Thread for handling the metadata requests
   std::thread m_imageThread; ///< Thread for publishing image slice updates

   zmq::context_t * m_ZMQ_context {nullptr}; ///< The ZeroMQ context, allocated on construction.

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
   
   /// Set the semaphore number to monitor
   /**
     * \returns 0 on success
     * \returns -1 on error
     */ 
   int semaphoreNumber( const int & number /**< [in] the semaphore number to monitor*/);
   
   /// Get the number of the semaphore to monitor
   /**
     * \returns the semaphore number, current value of m_sempahoreNumber
     */ 
   int semaphoreNumber();
   
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
   
private:
   ///Thread starter, called by metaThreadStart on thread construction.  Calls metaThreadExec.
   static void internal_metaThreadStart( milkzmqServer * mzs /**< [in] a pointer to a milkzmqServer instance (normally this) */);

public:
   /// Start the metadata thread.
   int metaThreadStart();

   /// Execute the metadata thread.
   void metaThreadExec();


private:
   ///Thread starter, called by imageThreadStart on thread construction.  Calls imageThreadExec.
   static void internal_imageThreadStart( milkzmqServer * mzs /**< [in] a pointer to a milkzmqServer instance (normally this) */);

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

bool milkzmqServer::m_timeToDie = false;

inline
milkzmqServer::milkzmqServer()
{
   
   m_ZMQ_context = new zmq::context_t;
}

inline
milkzmqServer::~milkzmqServer()
{
   if(m_ZMQ_context) delete m_ZMQ_context;
}

inline 
int milkzmqServer::argv0( const std::string & av0 )
{
   m_argv0 = av0;
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
   m_shMemImName = name;
   
   return 0;
}

inline
std::string milkzmqServer::shMemImName()
{
   return m_shMemImName;
}

inline
int milkzmqServer::semaphoreNumber( const int & number )
{
   m_sempahoreNumber = number;
   return 0;
}

inline
int milkzmqServer::semaphoreNumber()
{
   return m_sempahoreNumber;
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
void milkzmqServer::internal_metaThreadStart( milkzmqServer * mzs )
{
   mzs->metaThreadExec();
}

inline
int milkzmqServer::metaThreadStart()
{
   try
   {
      m_metaThread = std::thread( internal_metaThreadStart, this);
   }
   catch( const std::exception & e )
   {
      std::cerr << "milkzmqServer: exception in meta thread startup.\n";
      std::cerr << "  " <<  e.what() << "\n";
      std::cerr << "  at " __FILE__ << " line " << __LINE__ << "\n"; 
      return -1;
   }
   catch( ... )
   {
      std::cerr << "milkzmqServer: unknown exception in meta thread startup.\n";
      std::cerr << "  at " __FILE__ << " line " << __LINE__ << "\n"; 
      return -1;
   }
   
   if(!m_metaThread.joinable())
   {
      std::cerr << "milkzmqServer: meta thread did not start.\n";
      std::cerr << "  at " __FILE__ << " line " << __LINE__ << "\n";
      return -1;
   }
   
   return 0;
}

inline
void milkzmqServer::metaThreadExec()
{
   zmq::socket_t socket (*m_ZMQ_context, ZMQ_REP);
   socket.bind ("tcp://*:5555");

   while (true) 
   {
      zmq::message_t request;

      //  Wait for next request from client
      socket.recv (&request);
      std::cout << "Received Hello" << std::endl;

      //  Send reply back to client
      zmq::message_t reply (5);
      memcpy (reply.data (), "World", 5);
      socket.send (reply);
   }
    
}


inline
void milkzmqServer::internal_imageThreadStart( milkzmqServer * mzs )
{
   mzs->imageThreadExec();
}

inline
int milkzmqServer::imageThreadStart()
{
   try
   {
      m_imageThread = std::thread( internal_imageThreadStart, this);
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
   
   if(!m_imageThread.joinable())
   {
      reportError("image thread did not start", __FILE__, __LINE__);
      return -1;
   }
   
   return 0;
}

inline
void milkzmqServer::imageThreadExec()
{   
   IMAGE image;

   size_t type_size = 0; ///< The size, in bytes, of the image data type

   sem_t * sem {nullptr}; ///< The semaphore to monitor for new image data

   std::string srvstr = "tcp://*:" + std::to_string(m_imagePort);
   
   std::cout << "milkzmqServer: Beginning service at " << srvstr << "\n";
   
   zmq::socket_t publisher (*m_ZMQ_context, ZMQ_PUB);
   publisher.bind(srvstr);
   
   bool opened = false;
   
   uint8_t * msg = nullptr;
   
   while(!m_timeToDie)
   {
      opened = false;
      while(!opened && !m_timeToDie)
      {
         if( ImageStreamIO_openIm(&image, m_shMemImName.c_str()) == 0)
         {
            if(image.md[0].sem <= m_sempahoreNumber) 
            {
               ImageStreamIO_closeIm(&image);
               milkzmq::sleep(1); //We just need to wait for the server process to finish startup.
            }
            else
            {
               sem = image.semptr[m_sempahoreNumber];
               type_size = ImageStreamIO_typesize(image.md[0].atype);
               opened = true;
            }
         }
         else
         {
            milkzmq::sleep(1); //be patient
         }
      }
      
      
    
      int curr_image;
      uint8_t atype;
      size_t snx, sny, snz;
      uint8_t last_atype = image.md[0].atype;
      size_t last_snx = image.md[0].size[0];
      size_t last_sny = image.md[0].size[1];
      size_t last_snz = image.md[0].size[2];

      size_t msgSz = 128 + sizeof(uint8_t) + sizeof(uint64_t) + sizeof(uint64_t) + last_snx*last_sny*type_size;
      msg = (uint8_t *) malloc(msgSz);
      
      double lastCheck = get_curr_time();
      double lastSend = get_curr_time();
      double delta = 0;
      
      while(!m_timeToDie)
      {
         
         if(sem_trywait(sem) == 0)
         {
            
            if(image.md[0].size[2] > 0) ///\todo change to naxis?
            {
               curr_image = image.md[0].cnt1 - 1;
               if(curr_image < 0) curr_image = image.md[0].size[2] - 1;
            }
            else curr_image = 0;

            atype = image.md[0].atype;
            snx = image.md[0].size[0];
            sny = image.md[0].size[1];
            snz = image.md[0].size[2];
         
            if( atype!= last_atype || snx != last_snx || sny != last_sny || snz != last_snz )
            {
               break; //exit the nearest while loop and get the new image setup.
            }
         
            //Do a wait for max fps here.
            if( get_curr_time() - lastCheck < 1.0/m_fpsTgt-delta) 
            {
               milkzmq::microsleep(m_usecSleep);
               continue;
            }
            lastCheck = get_curr_time();

         
            memset(msg, 0, 128);
            snprintf((char *) msg, 128, "%s", m_shMemImName.c_str());
            *((uint8_t *) (msg + 128)) = atype;
            *((uint64_t *) (msg + 128 + sizeof(uint8_t))) = snx;
            *((uint64_t *) (msg + 128 + sizeof(uint8_t) + sizeof(uint64_t)) ) = sny;
            
            memcpy(msg + 128 + sizeof(uint8_t) + 2*sizeof(uint64_t), image.array.SI8 + curr_image*snx*sny*type_size, snx*sny*type_size);
                        
            publisher.send(msg, msgSz);
            
            double ct = get_curr_time();
            delta += m_fpsGain * (ct-lastSend - 1.0/m_fpsTgt);
            lastSend = ct;
            
         }
         else
         {
            if(errno != EAGAIN) break;

            if(image.md[0].sem <= 0) break; //Indicates that the server has cleaned up.
            
            milkzmq::microsleep(m_usecSleep);
         }
      }

      if(opened) 
      {
         ImageStreamIO_closeIm(&image);
         opened = false;
      }
      
      free(msg);
      msg = nullptr;
   }
   
   

   //One more check
   if(opened) ImageStreamIO_closeIm(&image);
   if(msg) free(msg);
   
} // milkzmqServer::imageThreadExec()

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
