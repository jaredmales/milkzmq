/** \file milk0Server.cpp
  * \brief Main function for a simple ZeroMQ ImageStreamIO server
  * \author Jared R. Males (jaredmales@gmail.com)
  *
  * History:
  * - 2018 created by JRM
  */

#include <signal.h>

#include "milk0Server.hpp"


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
   
   milk0::milk0Server::m_timeToDie = true;
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
   
   std::cerr << "usage: " << argv0 << " [options] shm-name\n\n";
   
   std::cerr << "   shm-name is the root of the ImageStreamIO shared memory image file.\n";
   std::cerr << "            If the full path is \"/tmp/image00.im.shm\" then shm-name=image00\n";
   std::cerr << "options:\n";
   std::cerr << "    -h    print this message and exit.\n";
   std::cerr << "    -p    specify the port number of the server [default = 5556].\n";
   std::cerr << "    -u    specify the loop sleep time in usecs [default = 100].\n";
   std::cerr << "    -f    specify the F.P.S. target [default = 30.0].\n";
   std::cerr << "    -s    specify the semaphore number [default=0].\n";
   
}

int main( int argc,
          char ** argv
        )
{
   
   int port = 5556;
   int usecSleep = 100;
   float fpsTgt = 30.0;
   int semNum = 0;

   bool help = false;

   argv0 = argv[0];
   
   opterr = 0;
   
   int c;
   while ((c = getopt (argc, argv, "hp:u:f:s:")) != -1)
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
         case 's':
            semNum = atoi(optarg);
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


   if( optind != argc-1)
   {
      usage("must specify shared memory file name as only non-option argument.");
      return -1;
   }
   
   std::string shmem_key = argv[optind];
   
   
   
   milk0::milk0Server mzs;
   
   mzs.argv0(argv0);
   mzs.imagePort(port);
   mzs.shMemImName(shmem_key);
   mzs.semaphoreNumber(semNum);
   mzs.fpsTgt(fpsTgt);
   mzs.usecSleep(usecSleep);
   
   mzs.imageThreadStart();
   
   while(1) 
   {
      milk0::sleep(1);
   }
}
