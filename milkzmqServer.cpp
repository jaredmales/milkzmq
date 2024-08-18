/** \file milkzmqServer.cpp
  * \brief Main function for a simple ZeroMQ ImageStreamIO server
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
#include <format>

#define XSTR(x) STR(x)
#define STR(x) #x

#define VALUE_TO_STRING(x) #x
#define VALUE(x) VALUE_TO_STRING(x)
#define VAR_NAME_VALUE(var) #var "="  VALUE(var)

#include "milkzmqServer.hpp"

std::string argv0;

void sigTermHandler( int signum,
                 siginfo_t *siginf,
                 void *ucont
               )
{
   //Suppress those warnings . . .
   static_cast<void>(signum);
   static_cast<void>(siginf);
   static_cast<void>(ucont);
  
   milkzmq::milkzmqServer::m_timeToDie = true;
}

void sigSegvHandler( int signum,
                 siginfo_t *siginf,
                 void *ucont
               )
{
   //Suppress those warnings . . .
   static_cast<void>(signum);
   static_cast<void>(siginf);
   static_cast<void>(ucont);

   milkzmq::milkzmqServer::m_restart = true;
}

int setSigTermHandler()
{
   struct sigaction act;
   sigset_t set;

   act.sa_sigaction = sigTermHandler;
   act.sa_flags = SA_SIGINFO;
   sigemptyset(&set);
   act.sa_mask = set;

   errno = 0;
   if( sigaction(SIGTERM, &act, 0) < 0 )
   {
      std::cerr << " (" << argv0 << "): error setting SIGTERM handler: " << strerror(errno) << "\n";
      return -1;
   }

   errno = 0;
   if( sigaction(SIGQUIT, &act, 0) < 0 )
   {
      std::cerr << " (" << argv0 << "): error setting SIGQUIT handler: " << strerror(errno) << "\n";
      return -1;
   }

   errno = 0;
   if( sigaction(SIGINT, &act, 0) < 0 )
   {
      std::cerr << " (" << argv0 << "): error setting SIGINT handler: " << strerror(errno) << "\n";
      return -1;
   }

   return 0;
}

int setSigSegvHandler()
{
   struct sigaction act;
   sigset_t set;

   act.sa_sigaction = sigSegvHandler;
   act.sa_flags = SA_SIGINFO;
   sigemptyset(&set);
   act.sa_mask = set;

   errno = 0;
   if( sigaction(SIGSEGV, &act, 0) < 0 )
   {
      std::cerr << " (" << argv0 << "): error setting SIGSEGV handler: " << strerror(errno) << "\n";
      return -1;
   }

   errno = 0;
   if( sigaction(SIGBUS, &act, 0) < 0 )
   {
      std::cerr << " (" << argv0 << "): error setting SIGBUS handler: " << strerror(errno) << "\n";
      return -1;
   }

   return 0;
}

void usage( const char * msg = 0 )
{
   std::cerr << argv0 << ": \n\n";
   
   if(msg) std::cerr << "error: " << msg << "\n\n";
   
   std::cerr << "usage: " << argv0 << " [options] shm-name [shm-names]\n\n";
   
   std::cerr << "   shm-name is the root of the ImageStreamIO shared memory image file(s).\n";
   std::cerr << "            If the full path is \"/tmp/image00.im.shm\" then shm-name=image00\n";
   std::cerr << "            At least one must be specified.\n";
   std::cerr << "options:\n";
   std::cerr << "    -h    print this message and exit.\n";
   std::cerr << "    -p    specify the port number of the server [default = 5556].\n";
   std::cerr << "    -u    specify the loop sleep time in usecs [default = 1000].\n";
   std::cerr << "    -f    specify the F.P.S. target [default = 10.0].\n";
   std::cerr << "    -x    turn on compression for INT16 and UINT16 types [default is off].\n";
   std::cerr << "    -a    If no shm-names are listed, export all from MILK_SHM_DIR.\n";
}

int main( int argc,
          char ** argv
        )
{
   
   int port = 5556;
   int usecSleep = 1000;
   float fpsTgt = 10.0;
   bool compress = false;
   bool exportAll = false;
   bool help = false;
   argv0 = argv[0];
   opterr = 0;
   int c;

   while ((c = getopt (argc, argv, "ahxp:u:f:")) != -1)
   {
      if(c == 'h')
      {
         help = true;
         break;
      }
      
      if( optarg != NULL)
      {
         if (optarg[0] == '-')
         {
            optopt = c;
            c = '?';
         }
      }
      
      switch (c)
      {
         case 'p':
            port = atoi(optarg);
            break;
         case 'u':
            usecSleep = atoi(optarg);
            break;
         case 'f':
           fpsTgt = atof(optarg);
           break;
         case 'x':
            compress = true;
            break;
         case '?':
            char errm[256];
            if (optopt == 'p' || optopt == 'u' || optopt == 'f' || optopt == 's')
               snprintf(errm, 256, "Option -%c requires an argument.", optopt);
            else if (isprint (optopt))
               snprintf(errm, 256, "Unknown option `-%c'.", optopt);
            else
               snprintf(errm, 256, "Unknown option character `\\x%x'.", optopt);

            usage(errm);
            return 1;
         case 'a':
            exportAll = true;
            break;
         default:
            usage(argv[0]);
            abort ();
      }
   }

   if(help)
   {
      usage();
      return 0;
   }


   if((!exportAll) && (optind > argc-1))
   {
      usage("must specify at least one shared memory file name as only non-option argument.");
      return -1;
   }
   
   std::list<std::string> streams;
   std::string shmDir;
   if(exportAll) {
      // Look for image streams.
      shmDir = std::string(std::getenv("MILK_SHM_DIR"));
      for (const auto &entry : std::filesystem::directory_iterator(shmDir)) {
         if (boost::algorithm::ends_with(entry.path().filename().generic_string(), ".im.shm")) {
            streams.push_back(entry.path().stem().stem().generic_string());
         }
      }

      // Log that we didn't find any.
      if (streams.size() == 0) {
         std::cerr << "I didn't find any image streams, but will wait to see if they're created." << std::endl;
      }
   }

   // Get everything ready.
   milkzmq::milkzmqServer mzs;
   mzs.argv0(argv0);
   mzs.imagePort(port);
   if(compress) mzs.defaultCompression();
   mzs.fpsTgt(fpsTgt);
   mzs.usecSleep(usecSleep);
   setSigTermHandler();
   setSigSegvHandler();
   
   // Start the explicitly named streams.
   for(int n=0; n < argc - optind; ++n) {
      mzs.shMemImName(argv[optind+n]);
   }
   
   // If we're doing them all, add the list.
   if(exportAll) {
      for(auto stream : streams) mzs.shMemImName(stream);
   }
 
   // Start the threads.
   mzs.serverThreadStart();
   size_t n = 0;
   for(; n < argc-optind; n++) mzs.imageThreadStart(n);
   for(; n < static_cast<int>(argc-optind + streams.size()); n++) mzs.imageThreadStart(n);

   // If we're exporting all image streams, use inotify to spawn threads for image streams as they're added.
   std::thread t_watcher;
   if(exportAll) {
      int inotify_fd = inotify_init();
      if (inotify_fd < 0) {
         std::perror("inotify_init");
      } else {
         // Tell inotify we want to know about new image streams.
         int retv = inotify_add_watch(inotify_fd, shmDir.c_str(), IN_CREATE);
         if (retv < 0) {
            std::perror("inotify_add_watch");
         }
      }
      
      // Use a lambda to wait for inotify events.
      t_watcher = std::thread([inotify_fd, &n, &mzs, shmDir]() {
         #if(__cpp_lib_format >= 202110L)
         std::cout << std::format("inotify->{}", shmDir) << std::endl;
         #endif
	 struct inotify_event event_buf[32];
         while (true) {
            std::size_t bytes_read = read(inotify_fd, &event_buf[0], sizeof(event_buf));  // Wait for events.
            if (bytes_read <= 0) {                                                        // Check to see we actually got something.
               std::perror("read on inotify_fd");                                         // If something terrible happened, give up.
               break;
            } else {
               int num = static_cast<int>(bytes_read / sizeof(struct inotify_event));
               for (int i = 0; i < num; i++) {                                                     // Try and process one or more events.
                  struct inotify_event *ie = event_buf + i;                                        // Get the event.
                  if (!(ie->mask & IN_CREATE)) continue;                                           // Don't both if it isn't a create event.
                  if (ie->len <= 0) continue;                                                      // If we don't have a file name, try another.
                  std::filesystem::path path(ie->name);                                            // Make a path for the created file.
                  if (boost::algorithm::ends_with(path.filename().generic_string(), ".im.shm")) {  // Is it an image stream?
                     mzs.shMemImName(path.stem().stem().generic_string());                         // It is. Start a thread for it.
                     mzs.imageThreadStart(n++);
                  }
               }
            }
         }
      });
   }

   // Wait until it's time for us to stop.   
   while(!milkzmq::milkzmqServer::m_timeToDie) milkzmq::sleep(1);
   
   // Stop everything.
   mzs.serverThreadKill();
   for(size_t m=0; m < n; n++) mzs.imageThreadKill(n);
}
