#ifndef SHOW_HPP
#define SHOW_HPP


#include <string>
#include <iterator>
#include <memory>
#include <sstream>
#include <map>
#include <vector>

#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>

// DEBUG:
#include <iostream>


namespace show
{
    typedef int socket_fd;
    
    struct request {
        std::string                                         method;
        std::vector< std::string >                          path;
        std::string                                         protocol;
        std::map< std::string, std::vector< std::string > > headers;
        std::stringstream                                   content;
    };
    
    template< typename R > class server
    {
    public:
        typedef std::shared_ptr< R > (* handler_t )( request& );
        
        server( handler_t, unsigned int );
        
        void serve();
        void serve( unsigned int );
        
    protected:
        handler_t handler;
        unsigned int port;
        socket_fd listen_socket;
        
        std::shared_ptr< R > current_response;
        
        virtual       bool         has_next_chunk();
        virtual const std::string& get_next_chunk();
    };
    
    typedef server< std::string > basic_server;
    
    // Common server implementations ///////////////////////////////////////////
    
    template< typename R > server< R >::server(
        server< R >::handler_t handler,
        unsigned int port
    ) :
        handler( handler ),
        port( port ),
        listen_socket( 0 ),
        current_response( nullptr )
    {}
    
    template< typename R > void server< R >::serve()
    {
        listen_socket = socket( AF_INET, SOCK_STREAM, 0 );
        
        if( listen_socket == 0 )
        {
            // DEBUG:
            perror( "Failed to create listen socket" );
            exit( -1 );
        }
        
        int opt = 1;
        
        if( setsockopt(
            listen_socket,
            SOL_SOCKET,
            SO_REUSEADDR | SO_REUSEPORT,
            &opt,
            sizeof( opt )
        ) )
        {
            // DEBUG:
            perror( "Failed to set listen socket options" );
            exit( -1 );
        }
        
        sockaddr_in address;
        address.sin_family      = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port        = htons( port );
        
        if( bind(
            listen_socket,
            ( sockaddr* )&address,
            sizeof( address )
        ) < 0 )
        {
            // DEBUG:
            perror( "Failed to bind listen socket" );
            exit( -1 );
        }
        
        // DEBUG:
        std::cout
            << "Serving forever on "
            << inet_ntoa( address.sin_addr )
            << ':'
            << ( int )ntohs( address.sin_port )
            << "...\n";
        
        while( true )
        {
            if( listen( listen_socket, 3 ) < 0 )
            {
                // DEBUG:
                perror( "Could not listen on socket" );
                exit( -1 );
            }
            
            // DEBUG:
            std::cout << "Listening...\n";
            
            socklen_t address_len = sizeof( address );
            
            socket_fd serve_socket = accept(
                listen_socket,
                ( sockaddr* )&address,
                ( socklen_t* )&address_len
            );
            
            if( serve_socket < 0 )
            {
                // DEBUG:
                perror( "Could not create serve socket" );
                exit( -1 );
            }
            
            // DEBUG:
            std::cout
                << "Got a request from "
                << inet_ntoa( address.sin_addr )
                << ':'
                << ( int )ntohs( address.sin_port )
                << " !\n";
            
            bool reading = true;
            char buffer[ 1024 ];
            int bytes_read;
            
            request req;
            
            // Start parsing headers with at lease one newline by definition
            int seq_newlines = 1;
            std::string current_header_name;
            bool new_header_value;
            
            // Parsing request assumes it is well-formed for now
            
            enum {
                READING_METHOD,
                READING_PATH,
                READING_PROTOCOL,
                READING_HEADER_NAME,
                READING_HEADER_VALUE,
                GETTING_CONTENT
            } parse_state = READING_METHOD;
            
            // DEBUG:
            std::cout << "Reading request...\n";
            
            while( reading )
            {
                bytes_read = read( serve_socket, buffer, 1023 );
                
                std::cout.write( buffer, bytes_read );
                
                for( int i = 0; reading && i < bytes_read; ++i )
                {
                    switch( parse_state )
                    {
                    case READING_METHOD:
                        {
                            switch( buffer[ i ] )
                            {
                            case ' ':
                                parse_state = READING_PATH;
                                break;
                            default:
                                req.method += buffer[ i ];
                                break;
                            }
                        }
                        break;
                    case READING_PATH:
                        {
                            switch( buffer[ i ] )
                            {
                            case ' ':
                                parse_state = READING_PROTOCOL;
                                break;
                            case '/':
                                if( req.path.size() > 0 )
                                    req.path.push_back( "" );
                                break;
                            default:
                                if( req.path.size() < 1 )
                                    req.path.push_back( std::string( buffer + i, 1 ) );
                                else
                                    req.path[ req.path.size() - 1 ] += buffer[ i ];
                                break;
                            }
                        }
                        break;
                    case READING_PROTOCOL:
                        {
                            switch( buffer[ i ] )
                            {
                            case '\r':
                                break;
                            case '\n':
                                parse_state = READING_HEADER_NAME;
                                break;
                            default:
                                req.protocol += buffer[ i ];
                                break;
                            }
                        }
                        break;
                    case READING_HEADER_NAME:
                        {
                            switch( buffer[ i ] )
                            {
                            case '\r':
                                break;
                            case '\n':
                                if( ++seq_newlines >= 2 )
                                {
                                    parse_state = GETTING_CONTENT;
                                    reading = false;
                                }
                                current_header_name = "";
                                break;
                            case ':':
                                seq_newlines = 0;
                                parse_state = READING_HEADER_VALUE;
                                new_header_value = true;
                                break;
                            default:
                                seq_newlines = 0;
                                current_header_name += buffer[ i ];
                                break;
                            }
                        }
                        break;
                    case READING_HEADER_VALUE:
                        {
                            std::vector< std::string >& header_values = req.headers[ current_header_name ];
                            
                            if( new_header_value )
                                header_values.push_back( "" );
                            
                            switch( buffer[ i ] )
                            {
                            case '\r':
                                break;
                            case '\n':
                                --i;
                                parse_state = READING_HEADER_NAME;
                                break;
                            default:
                                header_values[ header_values.size() - 1 ] += buffer[ i ];
                                break;
                            }
                            
                            new_header_value = false;
                        }
                        break;
                    case GETTING_CONTENT:
                        reading = false;
                        break;
                    }
                }
            }
            
            // DEBUG:
            std::cout << "Serving request...\n";
            
            current_response = handler( req );
            
            // DEBUG:
            std::cout << "Sending headers...\n";
            
            std::string response_headers( "200 OK\nContent-Type: text/plain\n\n" );
            send(
                serve_socket,
                response_headers.c_str(),
                response_headers.size(),
                0
            );
            
            while( has_next_chunk() )
            {
                const std::string& chunk( get_next_chunk() );
                send(
                    serve_socket,
                    chunk.c_str(),
                    chunk.size(),
                    0
                );
            }
            
            current_response = nullptr;
            
            close( serve_socket );
        }
    }
    // template< typename R > void server< R >::serve( unsigned int timeout )
    // {
    //     
    // }
    
    template< typename R > bool server< R >::has_next_chunk()
    {
        return *current_response != current_response -> end();
    }
    template< typename R > const std::string& server< R >::get_next_chunk()
    {
        return *( ( *current_response )++ );
    }
    
    // Server specializations //////////////////////////////////////////////////
    
    template<> bool server< std::string >::has_next_chunk()
    {
        return current_response != nullptr;
    }
    template<> const std::string& server< std::string >::get_next_chunk()
    {
        std::shared_ptr< std::string > s( current_response );
        current_response = nullptr;
        return *s;
    }
}


#endif
