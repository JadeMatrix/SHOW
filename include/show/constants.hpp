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
        // These are provided in PascalCase thanks to `continue` for code 100
        // being a reserved word
        
        // 1xx - Informational -------------------------------------------------
        
        static const response_code Continue                     { 100, "Continue"                        };
        static const response_code SwitchingProtocols           { 101, "Switching Protocols"             };
        static const response_code Processing                   { 102, "Processing"                      };
        
        // 2xx - Success -------------------------------------------------------
        
        static const response_code OK                           { 200, "OK"                              };
        static const response_code Created                      { 201, "Created"                         };
        static const response_code Accepted                     { 202, "Accepted"                        };
        static const response_code NonAuthoritativeInformation  { 203, "Non-Authoritative Information"   };
        static const response_code NoContent                    { 204, "No Content"                      };
        static const response_code ResetContent                 { 205, "Reset Content"                   };
        static const response_code PartialContent               { 206, "Partial Content"                 };
        static const response_code MultiStatus                  { 207, "Multi-Status"                    };
        static const response_code AlreadyReported              { 208, "Already Reported"                };
        static const response_code IMUsed                       { 226, "IM Used"                         };
        
        // 3xx - Redirection ---------------------------------------------------
        
        static const response_code MultipleChoices              { 300, "Multiple Choices"                };
        static const response_code MovedPermanently             { 301, "Moved Permanently"               };
        static const response_code Found                        { 302, "Found"                           };
        static const response_code SeeOther                     { 303, "See Other"                       };
        static const response_code NotModified                  { 304, "Not Modified"                    };
        static const response_code UseProxy                     { 305, "Use Proxy"                       };
        static const response_code SwitchProxy                  { 306, "Switch Proxy"                    };
        static const response_code TemporaryRedirect            { 307, "Temporary Redirect"              };
        static const response_code PermanentRedirect            { 308, "Permanent Redirect"              };
        
        // 4xx - Client Errors -------------------------------------------------
        
        static const response_code BadRequest                   { 400, "Bad Request"                     };
        static const response_code Unauthorized                 { 401, "Unauthorized"                    };
        static const response_code PaymentRequired              { 402, "Payment Required"                };
        static const response_code Forbidden                    { 403, "Forbidden"                       };
        static const response_code NotFound                     { 404, "Not Found"                       };
        static const response_code MethodNotAllowed             { 405, "Method Not Allowed"              };
        static const response_code NotAcceptable                { 406, "Not Acceptable"                  };
        static const response_code ProxyAuthenticationRequired  { 407, "Proxy Authentication Required"   };
        static const response_code RequestTimeout               { 408, "Request Timeout"                 };
        static const response_code Conflict                     { 409, "Conflict"                        };
        static const response_code Gone                         { 410, "Gone"                            };
        static const response_code LengthRequired               { 411, "Length Required"                 };
        static const response_code PreconditionFailed           { 412, "Precondition Failed"             };
        static const response_code PayloadTooLarge              { 413, "Payload Too Large"               };
        static const response_code URITooLong                   { 414, "URI Too Long"                    };
        static const response_code UnsupportedMediaType         { 415, "Unsupported Media Type"          };
        static const response_code RangeNotSatisfiable          { 416, "Range Not Satisfiable"           };
        static const response_code ExpectationFailed            { 417, "Expectation Failed"              };
        static const response_code Imateapot                    { 418, "I'm a teapot"                    };
        static const response_code MisdirectedRequest           { 421, "Misdirected Request"             };
        static const response_code UnprocessableEntity          { 422, "Unprocessable Entity"            };
        static const response_code Locked                       { 423, "Locked"                          };
        static const response_code FailedDependency             { 424, "Failed Dependency"               };
        static const response_code UpgradeRequired              { 426, "Upgrade Required"                };
        static const response_code PreconditionRequired         { 428, "Precondition Required"           };
        static const response_code TooManyRequests              { 429, "Too Many Requests"               };
        static const response_code RequestHeaderFieldsTooLarge  { 431, "Request Header Fields Too Large" };
        static const response_code UnavailableForLegalReasons   { 451, "Unavailable For Legal Reasons"   };
        
        // 5xx - Server Errors -------------------------------------------------
        
        static const response_code InternalServerError          { 500, "Internal Server Error"           };
        static const response_code NotImplemented               { 501, "Not Implemented"                 };
        static const response_code BadGateway                   { 502, "Bad Gateway"                     };
        static const response_code ServiceUnavailable           { 503, "Service Unavailable"             };
        static const response_code GatewayTimeout               { 504, "Gateway Timeout"                 };
        static const response_code HTTPVersionNotSupported      { 505, "HTTP Version Not Supported"      };
        static const response_code VariantAlsoNegotiates        { 506, "Variant Also Negotiates"         };
        static const response_code InsufficientStorage          { 507, "Insufficient Storage"            };
        static const response_code LoopDetected                 { 508, "Loop Detected"                   };
        static const response_code NotExtended                  { 510, "Not Extended"                    };
        static const response_code NetworkAuthenticationRequired{ 511, "Network Authentication Required" };
    }
}


#endif
