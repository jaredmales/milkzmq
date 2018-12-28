/** \file milk0Server.hpp
  * \brief Class implementing a ZeroMQ ImageStreamIO server
  * \author Jared R. Males (jaredmales@gmail.com)
  *
  * History:
  * - 2018 created by JRM
  */

#ifndef milk0Server_hpp
#define milk0Server_hpp

#include <zmq.hpp>

#include <ImageStreamIO.h>

#include "milk0Utils.hpp"

namespace milk0 
{


class milk0Server
{
protected:
   
   /** \name Configurable Parameters 
     *
     *@{
     */
   
   std::string m_argv0 {"milk0Server"}; ///< The invoked name, used for error messages.
   
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
   milk0Server();
   
   /// Destructor
   ~milk0Server();
   
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
   static void internal_metaThreadStart( milk0Server * mzs /**< [in] a pointer to a milk0Server instance (normally this) */);

public:
   /// Start the metadata thread.
   int metaThreadStart();

   /// Execute the metadata thread.
   void metaThreadExec();


private:
   ///Thread starter, called by imageThreadStart on thread construction.  Calls imageThreadExec.
   static void internal_imageThreadStart( milk0Server * mzs /**< [in] a pointer to a milk0Server instance (normally this) */);

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

bool milk0Server::m_timeToDie = false;

inline
milk0Server::milk0Server()
{
   
   m_ZMQ_context = new zmq::context_t;
}

inline
milk0Server::~milk0Server()
{
   if(m_ZMQ_context) delete m_ZMQ_context;
}

inline 
int milk0Server::argv0( const std::string & av0 )
{
   m_argv0 = av0;
   return 0;
}

inline 
std::string milk0Server::argv0()
{
   return m_argv0;
}

inline
int milk0Server::imagePort( const int & imagePort )
{
   m_imagePort = imagePort;
   
   return 0;
}

inline
int milk0Server::imagePort()
{
   return m_imagePort;
}


inline
int milk0Server::shMemImName( const std::string & name )
{
   m_shMemImName = name;
   
   return 0;
}

inline
std::string milk0Server::shMemImName()
{
   return m_shMemImName;
}

inline
int milk0Server::semaphoreNumber( const int & number )
{
   m_sempahoreNumber = number;
   return 0;
}

inline
int milk0Server::semaphoreNumber()
{
   return m_sempahoreNumber;
}
   
inline
int milk0Server::usecSleep( const int & usec )
{
   m_usecSleep = usec;
   return 0;
}

inline
int milk0Server::usecSleep()
{
   return m_usecSleep;
}

inline
int milk0Server::fpsTgt(const float & fps )
{
   m_fpsTgt = fps;
   return 0;
}

inline
float milk0Server::fpsTgt()
{
   return m_fpsTgt;
}
 
inline
int milk0Server::fpsGain(const float & gain )
{
   m_fpsGain = gain;
   return 0;
}

inline
float milk0Server::fpsGain()
{
   return m_fpsGain;
}

inline
void milk0Server::internal_metaThreadStart( milk0Server * mzs )
{
   mzs->metaThreadExec();
}

inline
int milk0Server::metaThreadStart()
{
   try
   {
      m_metaThread = std::thread( internal_metaThreadStart, this);
   }
   catch( const std::exception & e )
   {
      std::cerr << "milk0Server: exception in meta thread startup.\n";
      std::cerr << "  " <<  e.what() << "\n";
      std::cerr << "  at " __FILE__ << " line " << __LINE__ << "\n"; 
      return -1;
   }
   catch( ... )
   {
      std::cerr << "milk0Server: unknown exception in meta thread startup.\n";
      std::cerr << "  at " __FILE__ << " line " << __LINE__ << "\n"; 
      return -1;
   }
   
   if(!m_metaThread.joinable())
   {
      std::cerr << "milk0Server: meta thread did not start.\n";
      std::cerr << "  at " __FILE__ << " line " << __LINE__ << "\n";
      return -1;
   }
   
   return 0;
}

inline
void milk0Server::metaThreadExec()
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
void milk0Server::internal_imageThreadStart( milk0Server * mzs )
{
   mzs->imageThreadExec();
}

inline
int milk0Server::imageThreadStart()
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
void milk0Server::imageThreadExec()
{   
   IMAGE image;

   size_t type_size = 0; ///< The size, in bytes, of the image data type

   sem_t * sem {nullptr}; ///< The semaphore to monitor for new image data

   std::string srvstr = "tcp://*:" + std::to_string(m_imagePort);
   
   std::cout << "milk0Server: Beginning service at " << srvstr << "\n";
   
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
               milk0::sleep(1); //We just need to wait for the server process to finish startup.
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
            milk0::sleep(1); //be patient
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
               milk0::microsleep(m_usecSleep);
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
            
            milk0::microsleep(m_usecSleep);
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
   
} // milk0Server::imageThreadExec()

inline 
void milk0Server::reportError( const std::string & msg,
                                  const std::string & file,
                                  int line
                                )
{
   milk0::reportError(m_argv0, msg, file, line);
}

} //namespace milk0 

#endif //milk0Server_hpp
