#pragma once
#ifndef SHOW_TESTS_ASYNC_UTILS_HPP
#define SHOW_TESTS_ASYNC_UTILS_HPP


#include <show.hpp>

#include <string>
#include <thread>


std::thread send_request_async(
    std::string address,
    int port,
    const std::function< void( show::socket_fd ) >& request_feeder
);
void write_to_socket(
    show::socket_fd s,
    const std::string m
);
void handle_request(
    const std::string& request,
    const std::function< void( show::connection& ) >& handler_callback
);
void run_checks_against_request(
    const std::string& request,
    const std::function< void( show::request& ) >& checks_callback
);


#endif
