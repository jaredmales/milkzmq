/** \file mzmq_reconnect_resize_test.cpp
 * \brief Integration test for milkzmq reconnect and resize behavior.
 * \author Jared R. Males (jaredmales@gmail.com)
 *
 * This test launches milkzmqServer and milkzmqClient subprocesses, drives a
 * source ImageStreamIO stream through a resize, then restarts the server and
 * verifies the client-side stream continues updating.
 */

#include <ImageStreamIO/ImageStreamIO.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <chrono>
#include <csignal>
#include <cstring>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

namespace
{

struct Proc
{
    pid_t pid{ -1 };
};

struct LocalState
{
    bool found{ false };
    uint32_t nx{ 0 };
    uint32_t ny{ 0 };
    uint64_t cnt0{ 0 };
};

class SourceStream
{
   public:
    SourceStream() = default;

    ~SourceStream()
    {
        destroy();
    }

    bool create( const std::string &name, uint32_t nx, uint32_t ny, uint8_t datatype )
    {
        destroy();

        uint32_t imsize[3];
        imsize[0] = nx;
        imsize[1] = ny;
        imsize[2] = 0;

        if( ImageStreamIO_createIm( &m_image, name.c_str(), 2, imsize, datatype, 1, 0, 0 ) != 0 )
        {
            return false;
        }

        m_opened = true;
        return true;
    }

    void destroy()
    {
        if( m_opened )
        {
            ImageStreamIO_destroyIm( &m_image );
            m_opened = false;
        }
    }

    bool writeFrame()
    {
        if( !m_opened )
            return false;

        uint32_t nx = m_image.md[0].size[0];
        uint32_t ny = m_image.md[0].size[1];
        size_t bytes = static_cast<size_t>( nx ) * static_cast<size_t>( ny ) * sizeof( float );

        m_buffer.resize( static_cast<size_t>( nx ) * static_cast<size_t>( ny ) );
        for( size_t n = 0; n < m_buffer.size(); ++n )
        {
            m_buffer[n] = static_cast<float>( ( m_seed + n ) & 0xFFFF );
        }
        ++m_seed;

        m_image.md[0].write = 1;
        std::memcpy( m_image.array.raw, m_buffer.data(), bytes );
        m_image.md[0].write = 0;
        ++m_image.md[0].cnt0;
        clock_gettime( CLOCK_REALTIME, &m_image.md[0].writetime );
        ImageStreamIO_sempost( &m_image, -1 );

        return true;
    }

   private:
    IMAGE m_image{};
    bool m_opened{ false };
    std::vector<float> m_buffer;
    uint64_t m_seed{ 0 };
};

int waitForExit( pid_t pid, int timeoutSec )
{
    auto start = std::chrono::steady_clock::now();
    int status = 0;
    while( true )
    {
        pid_t rv = waitpid( pid, &status, WNOHANG );
        if( rv == pid )
            return 0;
        if( rv < 0 )
            return -1;

        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>( std::chrono::steady_clock::now() - start );
        if( elapsed.count() > timeoutSec )
            return -1;

        std::this_thread::sleep_for( std::chrono::milliseconds( 50 ) );
    }
}

void terminateProc( Proc &p )
{
    if( p.pid <= 0 )
        return;

    kill( p.pid, SIGTERM );
    if( waitForExit( p.pid, 4 ) != 0 )
    {
        kill( p.pid, SIGKILL );
        static_cast<void>( waitForExit( p.pid, 2 ) );
    }
    p.pid = -1;
}

Proc launchProc( const std::vector<std::string> &argv )
{
    Proc p;
    pid_t pid = fork();
    if( pid < 0 )
        return p;

    if( pid == 0 )
    {
        std::vector<char *> cargv;
        cargv.reserve( argv.size() + 1 );
        for( const auto &a : argv )
        {
            cargv.push_back( const_cast<char *>( a.c_str() ) );
        }
        cargv.push_back( nullptr );

        execv( cargv[0], cargv.data() );
        _exit( 127 );
    }

    p.pid = pid;
    return p;
}

LocalState probeLocal( const std::string &name )
{
    LocalState st;
    IMAGE image{};
    if( ImageStreamIO_openIm( &image, name.c_str() ) != 0 )
    {
        return st;
    }

    st.found = true;
    st.nx = image.md[0].size[0];
    st.ny = image.md[0].size[1];
    st.cnt0 = image.md[0].cnt0;

    ImageStreamIO_closeIm( &image );
    return st;
}

bool waitForState( const std::string &localName, uint32_t wantNx, uint32_t wantNy, uint64_t minCnt, int timeoutSec, SourceStream &src )
{
    auto start = std::chrono::steady_clock::now();
    while( true )
    {
        static_cast<void>( src.writeFrame() );
        std::this_thread::sleep_for( std::chrono::milliseconds( 20 ) );

        LocalState st = probeLocal( localName );
        if( st.found && st.nx == wantNx && st.ny == wantNy && st.cnt0 >= minCnt )
            return true;

        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>( std::chrono::steady_clock::now() - start );
        if( elapsed.count() > timeoutSec )
            return false;
    }
}

}  // namespace

int main()
{
    const int port = 5597;
    const std::string remoteName = "mzmq_test_remote_" + std::to_string( static_cast<long long>( getpid() ) );
    const std::string localName = "mzmq_test_local_" + std::to_string( static_cast<long long>( getpid() ) );

    Proc server;
    Proc client;
    SourceStream src;

    if( !src.create( remoteName, 32, 32, _DATATYPE_FLOAT ) )
    {
        std::cerr << "FAIL: could not create source stream\n";
        return 1;
    }

    server = launchProc( { "./milkzmqServer", "-p", std::to_string( port ), remoteName } );
    if( server.pid <= 0 )
    {
        std::cerr << "FAIL: could not launch server\n";
        return 1;
    }

    client = launchProc( { "./milkzmqClient", "-p", std::to_string( port ), "127.0.0.1", remoteName + "/" + localName } );
    if( client.pid <= 0 )
    {
        terminateProc( server );
        std::cerr << "FAIL: could not launch client\n";
        return 1;
    }

    if( !waitForState( localName, 32, 32, 10, 20, src ) )
    {
        terminateProc( client );
        terminateProc( server );
        std::cerr << "FAIL: initial stream did not connect/update\n";
        return 1;
    }

    if( !src.create( remoteName, 48, 16, _DATATYPE_FLOAT ) )
    {
        terminateProc( client );
        terminateProc( server );
        std::cerr << "FAIL: could not resize source stream\n";
        return 1;
    }

    if( !waitForState( localName, 48, 16, 10, 20, src ) )
    {
        terminateProc( client );
        terminateProc( server );
        std::cerr << "FAIL: resize did not propagate to client\n";
        return 1;
    }

    terminateProc( server );
    std::this_thread::sleep_for( std::chrono::milliseconds( 300 ) );

    server = launchProc( { "./milkzmqServer", "-p", std::to_string( port ), remoteName } );
    if( server.pid <= 0 )
    {
        terminateProc( client );
        std::cerr << "FAIL: could not relaunch server\n";
        return 1;
    }

    LocalState before = probeLocal( localName );
    uint64_t minReconnectCnt = before.found ? before.cnt0 + 10 : 10;
    if( !waitForState( localName, 48, 16, minReconnectCnt, 20, src ) )
    {
        terminateProc( client );
        terminateProc( server );
        std::cerr << "FAIL: client did not recover after server restart\n";
        return 1;
    }

    terminateProc( client );
    terminateProc( server );
    src.destroy();

    std::cout << "PASS: reconnect + resize test passed\n";
    return 0;
}
