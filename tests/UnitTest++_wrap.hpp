#pragma once
#ifndef SHOW_TESTS_UNITTESTPP_WRAP_HPP
#define SHOW_TESTS_UNITTESTPP_WRAP_HPP


#include <string>
#include <sstream>

#include <show.hpp>
#include <show/multipart.hpp>


std::string escape_seq( const std::string& s );

// Stream format operators for ues with UnitTest++ must be declared before
// including UnitTest++.h
std::ostream& operator<<( std::ostream& out, const show::http_protocol& v );
std::ostream& operator<<(
    std::ostream& out,
    const show::request::content_length_flag& v
);
std::ostream& operator<<(
    std::ostream& out,
    const std::vector< std::string >& v
);
std::ostream& operator<<( std::ostream& out, const show::query_args_type& v );
std::ostream& operator<<( std::ostream& out, const show::headers_type& v );
std::ostream& operator<<(
    std::ostream& out,
    const show::multipart::iterator& iter
);


#include <UnitTest++/UnitTest++.h>


#endif
