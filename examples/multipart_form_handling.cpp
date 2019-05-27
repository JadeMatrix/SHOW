#include <show.hpp>
#include <show/multipart.hpp>

#include <iostream> // std::cout
#include <iterator> // std::istreambuf_iterator
#include <sstream>
#include <string>   // std::string, std::to_string()


// Set a Server header to display the SHOW version
const show::headers_type::value_type server_header{
    "Server",
    {
        show::version::name
        + " v"
        + show::version::string
    }
};

const std::string form{
    R"###(<!doctype html>
<html>
    <head>
        <meta charset=utf-8>
        <title>Form Analyzer</title>
    </head>
    <body>
        <form action="/analyze" method="post" enctype="multipart/form-data">
            <div>
                <label for="file">File:</label>
                <input type="file" id="file" name="a_file"></input>
            </div>
            <div>
                <label for="comment">Comment:</label>
                <textarea id="comment" name="a_comment"></textarea>
            </div>
            <div>
                <button type="submit">Analyze</button>
            </div>
        </form>
    </body>
</html>
)###"
};


void get_form( show::request& request )
{
    show::response response{
        request.connection(),
        show::protocol::http_1_0,
        { 200, "OK" },
        {
            server_header,
            { "Content-Type"  , { "text/html"                   } },
            { "Content-Length", { std::to_string( form.size() ) } }
        }
    };
    
    response.sputn( form.c_str(), form.size() );
}

void analyze_form( show::request& request )
{
    auto found_content_type_header = request.headers().find( "Content-Type" );
    if(
        found_content_type_header == request.headers().end()
        || found_content_type_header -> second.size() != 1
        || found_content_type_header -> second[ 0 ].substr(
            0, found_content_type_header -> second[ 0 ].find( ";" )
        ) != "multipart/form-data"
    )
    {
        std::cout
            << "POST to analysis endpoint with improper/missing content type"
            << std::endl
        ;
        
        show::response response{
            request.connection(),
            show::protocol::http_1_0,
            { 400, "Bad Request" },
            { server_header }
        };
    }
    else
    {
        auto header_value   = found_content_type_header -> second[ 0 ];
        auto boundary_begin = header_value.find( "boundary=" ) + 9;
        auto boundary_end   = header_value.find( ";", boundary_begin );
        show::multipart parser{
            request,
            header_value.substr(
                boundary_begin,
                boundary_end
            )
        };
        
        std::stringstream analysis;
        analysis << "Form contains:\n";
        
        for( auto& segment : parser )
        {
            auto content = std::string{
                std::istreambuf_iterator< char >( &segment ),
                {}
            };
            analysis
                << " -  segment with size "
                << content.size()
                << " bytes, headers:\n"
            ;
            for( auto& header : segment.headers() )
                for( auto& value : header.second )
                    analysis
                        << "     -  "
                        << header.first
                        << ": "
                        << value
                        << "\n"
                    ;
            analysis
                << "    first eight bytes (URL-encoded): \""
                << show::url_encode( content.substr( 0, 8 ) )
                << "\"\n"
            ;
        }
        
        std::string result_html{
            "<!doctype html><html><head><meta charset=utf-8>"
            "<title>Result</title></head><body><pre>"
            + analysis.str()
            + "</pre></body></html>"
        };
        
        show::response response{
            request.connection(),
            show::protocol::http_1_0,
            { 200, "OK" },
            {
                server_header,
                { "Content-Type"  , { "text/html"                          } },
                { "Content-Length", { std::to_string( result_html.size() ) } }
            }
        };
        
        response.sputn( result_html.c_str(), result_html.size() );
    }
}


int main( int argc, char* argv[] )
{
    std::string  host   { "::" };   // IPv6 'any IP' (0.0.0.0 in IPv4)
    unsigned int port   { 9090 };   // Some random higher port
    int          timeout{ 10   };   // Connection timeout in seconds
    
    show::server test_server{
        host,
        port,
        timeout
    };
    
    while( true )
        try
        {
            show::connection connection{ test_server.serve() };
            
            try
            {
                show::request request{ connection };
                
                if(
                    request.path().size() == 1
                    && request.path()[ 0 ] == "form"
                )
                {
                    if( request.method() == "GET" )
                    {
                        get_form( request );
                        continue;
                    }
                }
                else if(
                    request.path().size() == 1
                    && request.path()[ 0 ] == "analyze"
                )
                {
                    if( request.method() == "POST" )
                    {
                        analyze_form( request );
                        continue;
                    }
                }
                else
                {
                    show::response response{
                        request.connection(),
                        show::protocol::http_1_0,
                        { 404, "Not Found" },
                        { server_header }
                    };
                    continue;
                }
                
                show::response response{
                    request.connection(),
                    show::protocol::http_1_0,
                    { 405, "Method Not Allowed" },
                    { server_header }
                };
            }
            catch( const show::connection_interrupted& ct )
            {
                std::cout
                    << "client "
                    << connection.client_address()
                    << " disconnected or timed out, closing connection"
                    << std::endl
                ;
            }
        }
        catch( const show::connection_timeout& ct )
        {
            std::cout
                << "timed out waiting for connection, looping..."
                << std::endl
            ;
        }
    
    return 0;
}
