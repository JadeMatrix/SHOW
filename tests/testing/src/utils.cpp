#include <show/testing/utils.hpp>

#include <random>


show::port_type random_port()
{
    std::random_device rd;
    std::minstd_rand gen{ rd() };
    return std::uniform_int_distribution< show::port_type >{
        49152,
        65535
    }( gen );
}
