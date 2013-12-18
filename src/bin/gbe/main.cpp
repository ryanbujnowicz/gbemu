#include <fstream>
#include <iomanip>
#include <iostream>

#include <boost/program_options.hpp>
namespace po = boost::program_options;

#include <cpu/cpu.h>
#include <cpu/memory.h>

const std::string ProgramName = "gbe";

void parseOptions(int argc, char** argv, po::variables_map& vm)
{
    po::positional_options_description p;
    p.add("input-rom", -1);

    po::options_description desc("Allowed options");     
    desc.add_options()
        ("help,h", "Prints this help message.")
        ("verbose,v", "Enables verbose output.")
        ("input-rom", "Input ROM to execute in emulator.")
        ("dump-registers", "Dumps the contents of CPU registers.")
        ("dump-memory", "Dumps the contents of memory.")
        ;

    po::store(po::command_line_parser(argc, argv).
        options(desc).
        positional(p).
        run(), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << ProgramName << ": GameBoy emulator" << std::endl << std::endl;
        std::cout << "Usage: " << ProgramName << " [options] file" << std::endl << std::endl;
        std::cout << desc << std::endl;
        exit(1);
    }
}

void errorAndExit(const std::string& err)
{
    std::cerr << "Error: " << err << std::endl;
    exit(0);
}

void loadRom(const std::string& romFile, gb::Memory* memory, bool verbose)
{
    std::ifstream fin(romFile, std::ios_base::binary);
    if (!fin) {
        errorAndExit("rom-file does not exist.");
    }

    // ROM file can be copied directly into the emulator's memory
    fin.seekg(0, fin.end);
    int length = fin.tellg();
    fin.seekg(0, fin.beg);

    if (verbose) {
        std::cout << "Reading " << length << " bytes from " << romFile << " into memory" 
                  << std::endl;
    }

    char* buffer = new char[length];
    fin.read(buffer, length);

    // Might not be fully correct, the real device will have randomized memory on
    // start but it sure helps with debugging to have it zero'd. 
    for (size_t i = 0; i < memory->size(); ++i) {
        (*memory)[i] = 0x00;
    }

    for (size_t i = 0; i < length; ++i) {
        (*memory)[i] = buffer[i];
    }

    delete[] buffer;
}

void execLoop(gb::Cpu& cpu, gb::Memory& memory, bool verbose)
{
    while (!cpu.isStopped()) {
        cpu.processNextInstruction(); 
    }

    if (verbose) {
        std::cout << "STOP instruction encountered" << std::endl;
    }
}

std::string registerToString(gb::Word word)
{
    std::stringstream ss;
    ss << "0x" << std::hex << std::setfill('0') << std::setw(4) << word;
    return ss.str();
}

std::string flagsToString(gb::Cpu& cpu)
{
    std::stringstream ss;

    for (gb::Cpu::Flag f: {gb::Cpu::FlagZ, gb::Cpu::FlagN, gb::Cpu::FlagH, gb::Cpu::FlagC}) {
        if (cpu.flag(f)) {
            ss << "1";
        } else {
            ss << "-";
        }
    }
    
    return ss.str();
}

void dumpRegisters(gb::Cpu& cpu)
{
    gb::Cpu::Registers regs = cpu.registers();
    std::cout << "AF: " << registerToString(regs.AF) << " "
              << "BC: " << registerToString(regs.BC) << " "
              << "DE: " << registerToString(regs.DE) << " "
              << "HL: " << registerToString(regs.HL) << " "
              << "SP: " << registerToString(regs.SP) << " "
              << "PC: " << registerToString(regs.PC) << " "
              << "F: " << flagsToString(cpu) << std::endl;
}

void dumpMemory(gb::Memory& memory)
{
    const int ChunkSize = 8;

    int i = 0;
    while (i < memory.size()) {
        std::cout << "0x" << std::hex << std::setfill('0') << std::setw(4) << i << "    ";

        for (int j = 0; j < ChunkSize; ++j) {
            std::cout << " " << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(memory[i + j]);
        }

        std::cout << std::endl;
        i += ChunkSize;
    }
}

int main(int argc, char** argv)
{
    po::variables_map vm;
    parseOptions(argc, argv, vm);

    if (!vm.count("input-rom")) {
        errorAndExit("must specify an input rom.");
    }

    bool verbose = vm.count("verbose");

    gb::Memory memory(32);
    gb::Cpu cpu;
    cpu.setMemory(&memory);

    loadRom(vm["input-rom"].as<std::string>(), &memory, verbose);
    execLoop(cpu, memory, verbose);

    if (vm.count("dump-registers")) {
        dumpRegisters(cpu);
    }
    if (vm.count("dump-memory")) {
        dumpMemory(memory);
    }
}

