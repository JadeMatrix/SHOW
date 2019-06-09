#pragma once
#ifndef SHOW_TESTING_DOCTEST_WRAP_HPP
#define SHOW_TESTING_DOCTEST_WRAP_HPP


#include <string>
#include <sstream>

#include <show.hpp>
#include <show/multipart.hpp>


std::string escape_seq( const std::string& s );

// Stream format operators for ues with doctest must be declared before
// including doctest/doctest.h
std::ostream& operator<<( std::ostream& out, const show::protocol& v );
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


#include <doctest/doctest.h>


#endif
