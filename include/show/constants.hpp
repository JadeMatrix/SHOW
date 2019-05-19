#pragma once
#ifndef SHOW_CONSTANTS_HPP
#define SHOW_CONSTANTS_HPP


#include "../show.hpp"


namespace show
{
    // Addresses & Ports ///////////////////////////////////////////////////////
    
    
    static const std::string  any_ip     { "::"        };
    static const std::string  loopback_ip{ "::1"       };
    static const std::string& localhost  { loopback_ip };
    
    // As SHOW is intended to be used behind a reverse proxy, these are of
    // questionable utility but still provided for educational purposes
    static const unsigned int http_port { 80   };
    static const unsigned int https_port{ 8080 };
    
    
    // Headers /////////////////////////////////////////////////////////////////
    
    
    // A default Server header to display the SHOW version.  If you implement a
    // higher-level HTTP framework with SHOW, you may want to instead set the
    // Server header to
    // "Your Server Framework vX.Y.Z using " + show::server_string
    static const std::string server_string{
        show::version::name
        + " v"
        + show::version::string
    };
    static const headers_type::value_type server_header{
        "Server", { server_string }
    };
    
    // Fallback MIME type recommended by RFC 2616 §7.2.1
    // https://www.w3.org/Protocols/rfc2616/rfc2616-sec7.html#sec7.2.1
    static const std::string default_content_type{ "application/octet-stream" };
    static const headers_type::value_type default_content_type_header{
        "Content-Type",
        { default_content_type }
    };
    
    
    // HTTP Methods ////////////////////////////////////////////////////////////
    
    
    namespace method
    {
        static const std::string GET    { "GET"     };
        static const std::string HEAD   { "HEAD"    };
        static const std::string POST   { "POST"    };
        static const std::string PUT    { "PUT"     };
        static const std::string DELETE { "DELETE"  };
        static const std::string TRACE  { "TRACE"   };
        static const std::string OPTIONS{ "OPTIONS" };
        static const std::string CONNECT{ "CONNECT" };
        static const std::string PATCH  { "PATCH"   };
    }
    
    
    // HTTP Response Codes /////////////////////////////////////////////////////
    
    
    namespace code
    {
        // 1xx - Informational -------------------------------------------------
        
        static const response_code CONTINUE                       { 100, "Continue"                        };
        static const response_code SWITCHING_PROTOCOLS            { 101, "Switching Protocols"             };
        static const response_code PROCESSING                     { 102, "Processing"                      };
        
        // 2xx - Success -------------------------------------------------------
        
        static const response_code OK                             { 200, "OK"                              };
        static const response_code CREATED                        { 201, "Created"                         };
        static const response_code ACCEPTED                       { 202, "Accepted"                        };
        static const response_code NON_AUTHORITATIVE_INFORMATION  { 203, "Non-Authoritative Information"   };
        static const response_code NO_CONTENT                     { 204, "No Content"                      };
        static const response_code RESET_CONTENT                  { 205, "Reset Content"                   };
        static const response_code PARTIAL_CONTENT                { 206, "Partial Content"                 };
        static const response_code MULTI_STATUS                   { 207, "Multi-Status"                    };
        static const response_code ALREADY_REPORTED               { 208, "Already Reported"                };
        static const response_code IM_USED                        { 226, "IM Used"                         };
        
        // 3xx - Redirection ---------------------------------------------------
        
        static const response_code MULTIPLE_CHOICES               { 300, "Multiple Choices"                };
        static const response_code MOVED_PERMANENTLY              { 301, "Moved Permanently"               };
        static const response_code FOUND                          { 302, "Found"                           };
        static const response_code SEE_OTHER                      { 303, "See Other"                       };
        static const response_code NOT_MODIFIED                   { 304, "Not Modified"                    };
        static const response_code USE_PROXY                      { 305, "Use Proxy"                       };
        static const response_code SWITCH_PROXY                   { 306, "Switch Proxy"                    };
        static const response_code TEMPORARY_REDIRECT             { 307, "Temporary Redirect"              };
        static const response_code PERMANENT_REDIRECT             { 308, "Permanent Redirect"              };
        
        // 4xx - Client Errors -------------------------------------------------
        
        static const response_code BAD_REQUEST                    { 400, "Bad Request"                     };
        static const response_code UNAUTHORIZED                   { 401, "Unauthorized"                    };
        static const response_code PAYMENT_REQUIRED               { 402, "Payment Required"                };
        static const response_code FORBIDDEN                      { 403, "Forbidden"                       };
        static const response_code NOT_FOUND                      { 404, "Not Found"                       };
        static const response_code METHOD_NOT_ALLOWED             { 405, "Method Not Allowed"              };
        static const response_code NOT_ACCEPTABLE                 { 406, "Not Acceptable"                  };
        static const response_code PROXY_AUTHENTICATION_REQUIRED  { 407, "Proxy Authentication Required"   };
        static const response_code REQUEST_TIMEOUT                { 408, "Request Timeout"                 };
        static const response_code CONFLICT                       { 409, "Conflict"                        };
        static const response_code GONE                           { 410, "Gone"                            };
        static const response_code LENGTH_REQUIRED                { 411, "Length Required"                 };
        static const response_code PRECONDITION_FAILED            { 412, "Precondition Failed"             };
        static const response_code PAYLOAD_TOO_LARGE              { 413, "Payload Too Large"               };
        static const response_code URI_TOO_LONG                   { 414, "URI Too Long"                    };
        static const response_code UNSUPPORTED_MEDIA_TYPE         { 415, "Unsupported Media Type"          };
        static const response_code RANGE_NOT_SATISFIABLE          { 416, "Range Not Satisfiable"           };
        static const response_code EXPECTATION_FAILED             { 417, "Expectation Failed"              };
        static const response_code IM_A_TEAPOT                    { 418, "I'm a teapot"                    };
        static const response_code MISDIRECTED_REQUEST            { 421, "Misdirected Request"             };
        static const response_code UNPROCESSABLE_ENTITY           { 422, "Unprocessable Entity"            };
        static const response_code LOCKED                         { 423, "Locked"                          };
        static const response_code FAILED_DEPENDENCY              { 424, "Failed Dependency"               };
        static const response_code UPGRADE_REQUIRED               { 426, "Upgrade Required"                };
        static const response_code PRECONDITION_REQUIRED          { 428, "Precondition Required"           };
        static const response_code TOO_MANY_REQUESTS              { 429, "Too Many Requests"               };
        static const response_code REQUEST_HEADER_FIELDS_TOO_LARGE{ 431, "Request Header Fields Too Large" };
        static const response_code UNAVAILABLE_FOR_LEGAL_REASONS  { 451, "Unavailable For Legal Reasons"   };
        
        // 5xx - Server Errors -------------------------------------------------
        
        static const response_code INTERNAL_SERVER_ERROR          { 500, "Internal Server Error"           };
        static const response_code NOT_IMPLEMENTED                { 501, "Not Implemented"                 };
        static const response_code BAD_GATEWAY                    { 502, "Bad Gateway"                     };
        static const response_code SERVICE_UNAVAILABLE            { 503, "Service Unavailable"             };
        static const response_code GATEWAY_TIMEOUT                { 504, "Gateway Timeout"                 };
        static const response_code HTTP_VERSION_NOT_SUPPORTED     { 505, "HTTP Version Not Supported"      };
        static const response_code VARIANT_ALSO_NEGOTIATES        { 506, "Variant Also Negotiates"         };
        static const response_code INSUFFICIENT_STORAGE           { 507, "Insufficient Storage"            };
        static const response_code LOOP_DETECTED                  { 508, "Loop Detected"                   };
        static const response_code NOT_EXTENDED                   { 510, "Not Extended"                    };
        static const response_code NETWORK_AUTHENTICATION_REQUIRED{ 511, "Network Authentication Required" };
    }
}


#endif
