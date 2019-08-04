/** \file milkzmqClient.cpp
  * \brief Main function for a simple ZeroMQ ImageStreamIO client
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
//r
// milkzmq is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with milkzmq.  If not, see <http://www.gnu.org/licenses/>.
//***********************************************************************//

#include <signal.h>

#include "milkzmqClient.hpp"

std::string argv0;

void sigHandler( int signum,
                 siginfo_t *siginf,
                 void *ucont
               )
{
   //Suppress those warnings . . .
   static_cast<void>(signum);
   static_cast<void>(siginf);
   static_cast<void>(ucont);
   
   milkzmq::milkzmqClient::m_timeToDie = true;
}

int setSigTermHandler()
{
   struct sigaction act;
   sigset_t set;

   act.sa_sigaction = sigHandler;
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

void usage( const char * msg = 0 )
{
   std::cerr << argv0 << ": \n\n";
   
   if(msg) std::cerr << "error: " << msg << "\n\n";
   
   std::cerr << "usage: " << argv0 << " [options] remote-host shm-name\n\n";
   
   std::cerr << "   remote-host is the address of the remote host where milkzmqServer is running.\n\n";
   std::cerr << "   shm-name is the root of the ImageStreamIO shared memory image file.\n";
   std::cerr << "            If the full path is \"/tmp/image00.im.shm\" then shm-name=image00\n";
   std::cerr << "options:\n";
   std::cerr << "    -h    print this message and exit.\n";
   std::cerr << "    -p    specify the port number of the server [default = 5556].\n";

   return;
}

int parseName( std::string & remName,
               std::string & locName,
               const std::string & name
             )
{
   size_t slash = name.find("/");
   
   if(slash == std::string::npos)
   {
      remName = name;
      locName = "";
      return 0;
   }
   
   if(slash == 0)
   {
      std::cerr << "invalid name specification (no remote): " << name << "\n";
      remName = "";
      locName = "";
      return -1;
   }
   
   remName = name.substr(0, slash);
   
   if(slash > name.size()-1)
   {
      locName = "";
      return 0;
   }
   
   locName = name.substr(slash+1);
   
   return 0;
}

int main (int argc, char *argv[])
{
   int port = 5556;
   bool help = false;

   argv0 = argv[0];
   
   opterr = 0;
   
   int c;
   while ((c = getopt (argc, argv, "hp:")) != -1)
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


   if( optind != argc-2)
   {
      usage("must specify remote address and shared memory file name as only non-option arguments.");
      return -1;
   }
   
   std::string remote_address = argv[optind];
   std::string shmem_key = argv[optind+1];
      
   
   milkzmq::milkzmqClient mzc;
   mzc.argv0(argv0);
   mzc.address(remote_address);
   mzc.imagePort(port);
   
   for(int n=0; n < argc - optind; ++n)
   {
      std::string remName, locName;
      
      if(parseName( remName, locName, argv[optind+n]) < 0)
      {
         usage();
         return -1;
      }
      
      mzc.shMemImName(remName, locName);
   }
   
   
   
   setSigTermHandler();
   
   for(size_t n=0; n < argc-optind; ++n)
   {
      mzc.imageThreadStart(n);
   }
   
   while(!milkzmq::milkzmqClient::m_timeToDie) 
   {
      milkzmq::sleep(1);
   }
   
   for(size_t n=0; n < argc-optind; ++n)
   {
      mzc.imageThreadKill(n);
   }
   

   return 0;
}
