/** \file milkzmqServer.cpp
  * \brief Main function for a simple ZeroMQ ImageStreamIO server
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

#include <signal.h>

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
   
   std::cerr << "usage: " << argv0 << " [options] shm-name [shm-name]\n\n";
   
   std::cerr << "   shm-name is the root of the ImageStreamIO shared memory image file(s).\n";
   std::cerr << "            If the full path is \"/tmp/image00.im.shm\" then shm-name=image00\n";
   std::cerr << "            At least one must be specified.\n";
   std::cerr << "options:\n";
   std::cerr << "    -h    print this message and exit.\n";
   std::cerr << "    -p    specify the port number of the server [default = 5556].\n";
   std::cerr << "    -u    specify the loop sleep time in usecs [default = 1000].\n";
   std::cerr << "    -f    specify the F.P.S. target [default = 10.0].\n";
   
}

int main( int argc,
          char ** argv
        )
{
   
   int port = 5556;
   int usecSleep = 1000;
   float fpsTgt = 10.0;
   
   bool help = false;

   argv0 = argv[0];
   
   opterr = 0;
   
   int c;
   while ((c = getopt (argc, argv, "hp:u:f:")) != -1)
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


   if( optind > argc-1)
   {
      usage("must specify at least one shared memory file name as only non-option argument.");
      return -1;
   }
   
   
   milkzmq::milkzmqServer mzs;
   
   mzs.argv0(argv0);
   mzs.imagePort(port);
   
   for(int n=0; n < argc - optind; ++n)
   {
      mzs.shMemImName(argv[optind+n]);
   }
   
   mzs.fpsTgt(fpsTgt);
   mzs.usecSleep(usecSleep);
  
   setSigTermHandler();
   setSigSegvHandler();
 
   mzs.serverThreadStart();

   for(size_t n=0; n < argc-optind; ++n)
   {
      mzs.imageThreadStart(n);
   }
   
   while(!milkzmq::milkzmqServer::m_timeToDie) 
   {
      milkzmq::sleep(1);
   }
}
