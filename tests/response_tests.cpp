#include "UnitTest++_wrap.hpp"
#include <show.hpp>

#include "async_utils.hpp"
#include "constants.hpp"


SUITE( ShowResponseTests )
{
    TEST( ReturnHTTP_1_0 )
    {
        // http_protocol::HTTP_1_0 -> HTTP/1.0
    }
    
    TEST( ReturnHTTP_1_1 )
    {
        // http_protocol::HTTP_1_1 -> HTTP/1.1
    }
    
    TEST( ReturnHTTP_1_0OnNone )
    {
        // http_protocol::NONE -> HTTP/1.0
    }
    
    TEST( ReturnHTTP_1_0OnUnknown )
    {
        // http_protocol::UNKNOWN -> HTTP/1.0
    }
    
    TEST( StandardResponseCode )
    {
        // standard response code
    }
    
    TEST( CustomResponseCode )
    {
        // custom response code
    }
    
    TEST( CustomFourDigitResponseCode )
    {
        // custom >3-digit response code
    }
    
    TEST( MultiLineHeader )
    {
        // header values with newlines -> multi-line header (failing)
    }
    
    TEST( FlushOnDestroy )
    {
        // flush on destroy
    }
    
    TEST( GracefulClientDisconnectWhileCreating )
    {
        // client disconnected while creating (probably failing - destructor will throw)
    }
    
    TEST( GracefulClientDisconnectWhileSending )
    {
        // client disconnected while sending (probably failing - destructor will throw)
    }
    
    TEST( GracefulClientDisconnectWhileDestroying )
    {
        // client disconnected while destroying
    }
    
    TEST( GracefulConnectionTimeoutWhileCreating )
    {
        // connection timeout while creating (probably failing - destructor will throw)
    }
    
    TEST( GracefulConnectionTimeoutWhileSending )
    {
        // connection timeout while sending (probably failing - destructor will throw)
    }
    
    TEST( GracefulConnectionTimeoutWhileDestroying )
    {
        // connection timeout while destroying
    }
    
    TEST( FailReturnInvalidHeaderName )
    {
        // fail on invalid header names (failing)
    }
}
