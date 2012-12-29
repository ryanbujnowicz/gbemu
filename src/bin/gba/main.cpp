#include <iostream>

#include <boost/program_options.hpp>
namespace po = boost::program_options;

void parseOptions(int argc, char** argv, po::variables_map& vm)
{
    po::options_description desc("Allowed options");     
    desc.add_options()
        ("help,h", "Prints this help message.");

    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << desc << std::endl;
        exit(1);
    }
}

int main(int argc, char** argv)
{
    po::variables_map vm;
    parseOptions(argc, argv, vm);
}

