/** \file milk0Client.hpp
  * \brief Class implementing a ZeroMQ ImageStreamIO client
  * \author Jared R. Males (jaredmales@gmail.com)
  *
  * History:
  * - 2018 created by JRM
  */

#ifndef milk0Client_hpp
#define milk0Client_hpp

#include <zmq.hpp>

#include <ImageStreamIO.h>

#include "milk0Utils.hpp"

namespace milk0 
{
   
class milk0Client
{
protected:
   
   /** \name Configurable Parameters 
     *
     *@{
     */
   
   std::string m_argv0 {"milk0Client"}; ///< The invoked name, used for error messages.
   
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
   milk0Client();
   
   /// Destructor
   ~milk0Client();
   
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
   static void internal_imageThreadStart( milk0Client * mzs /**< [in] a pointer to a milk0Client instance (normally this) */);

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

bool milk0Client::m_timeToDie = false;

inline
milk0Client::milk0Client()
{
   
   m_ZMQ_context = new zmq::context_t(1);
}

inline
milk0Client::~milk0Client()
{
   if(m_ZMQ_context) delete m_ZMQ_context;
}

inline 
int milk0Client::argv0( const std::string & av0 )
{
   m_argv0 = av0;
   return 0;
}

inline 
std::string milk0Client::argv0()
{
   return m_argv0;
}

inline 
int milk0Client::address( const std::string & add )
{
   m_address= add;
   return 0;
}

inline 
std::string milk0Client::address()
{
   return m_address;
}

inline
int milk0Client::imagePort( const int & imagePort )
{
   m_imagePort = imagePort;
   
   return 0;
}

inline
int milk0Client::imagePort()
{
   return m_imagePort;
}

inline
int milk0Client::shMemImName( const std::string & name )
{
   m_shMemImName = name;
   
   return 0;
}

inline
std::string milk0Client::shMemImName()
{
   return m_shMemImName;
}

inline
int milk0Client::localShMemImName( const std::string & name )
{
   m_localShMemImName = name;
   
   return 0;
}

inline
std::string milk0Client::localShMemImName()
{
   return m_localShMemImName;
}

inline
void milk0Client::internal_imageThreadStart( milk0Client * mzc )
{
   mzc->imageThreadExec();
}

inline
int milk0Client::imageThreadStart()
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
void milk0Client::imageThreadExec()
{   
   //size_t type_size = 0; ///< The size, in bytes, of the image data type

   std::string srvstr = "tcp://" + m_address + ":" + std::to_string(m_imagePort);
   
   std::cout << "milk0Client: Beginning receive at " << srvstr << "\n";
   
   zmq::socket_t subscriber (*m_ZMQ_context, ZMQ_SUB);
   subscriber.connect(srvstr);
      
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
        
      subscriber.recv(&msg);
      char * raw_image= (char *) msg.data();
      
      new_atype = *( (uint8_t *) (raw_image + 128) );
      new_nx = *( (uint64_t *) (raw_image + 128 + sizeof(uint8_t)));
      new_ny = *( (uint64_t *) (raw_image + 128  + sizeof(uint8_t) + sizeof(uint64_t)));
      
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
      
      size_t type_size = ImageStreamIO_typesize(image.md[0].atype);

      image.md[0].write=1;
      
      memcpy(image.array.SI8 + curr_image*nx*ny*type_size, raw_image + 128 + sizeof(uint8_t) + 2*sizeof(uint64_t), nx*ny*type_size);
      ImageStreamIO_sempost(&image,-1);
        
      image.md[0].write=0;

      image.md[0].cnt0++;
      image.md[0].cnt1=0;
   }

   if(opened) ImageStreamIO_closeIm(&image);
   
   
} // milk0Client::imageThreadExec()

inline 
void milk0Client::reportError( const std::string & msg,
                                  const std::string & file,
                                  int line
                                )
{
   milk0::reportError(m_argv0, msg, file, line);
}

} //namesapce milk0 

#endif //milk0Client_hpp
