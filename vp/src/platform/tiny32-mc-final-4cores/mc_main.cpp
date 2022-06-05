/*
	4 cores -- 4 sorters
*/


#include <cstdlib>
#include <ctime>

#include "core/common/clint.h"
#include "elf_loader.h"
#include "iss.h"
#include "mem.h"
#include "memory.h"
#include "syscall.h"
#include "platform/common/options.h"

#include "gdb-mc/gdb_server.h"
#include "gdb-mc/gdb_runner.h"

#include <boost/io/ios_state.hpp>
#include <boost/program_options.hpp>
#include <iomanip>
#include <iostream>

#include "Sorter.h"
#include "dma.h"
#include "fe310_plic.h"

using namespace rv32;
namespace po = boost::program_options;

struct TinyOptions : public Options {
public:
	typedef unsigned int addr_t;

	addr_t mem_size = 1024 * 1024 * 32;  // 32 MB ram, to place it before the CLINT and run the base examples (assume
	                                     // memory start at zero) without modifications
	addr_t mem_start_addr = 0x00000000;
	addr_t mem_end_addr = mem_start_addr + mem_size - 1;
	addr_t clint_start_addr = 0x02000000;
	addr_t clint_end_addr = 0x0200ffff;
	addr_t sys_start_addr = 0x02010000;
	addr_t sys_end_addr = 0x020103ff;

	addr_t plic_start_addr = 0x40000000;
	addr_t plic_end_addr = 0x41000000;
	addr_t dma0_start_addr = 0x70000000;
	addr_t dma0_end_addr = 0x70001000;
	addr_t dma1_start_addr = 0x70002000;
	addr_t dma1_end_addr = 0x70003000;
	addr_t dma2_start_addr = 0x70004000;
	addr_t dma2_end_addr = 0x70005000;
	addr_t dma3_start_addr = 0x70006000;
	addr_t dma3_end_addr = 0x70007000;
	addr_t sorter_size = 0x01000000;
	addr_t sorter_start_addr0 = 0x73000000;
 	addr_t sorter_end_addr0 = sorter_start_addr0 + sorter_size - 1;
	addr_t sorter_start_addr1 = 0x74000000;
 	addr_t sorter_end_addr1 = sorter_start_addr1 + sorter_size - 1;
	addr_t sorter_start_addr2 = 0x75000000;
 	addr_t sorter_end_addr2 = sorter_start_addr2 + sorter_size - 1;
	addr_t sorter_start_addr3 = 0x76000000;
 	addr_t sorter_end_addr3 = sorter_start_addr3 + sorter_size - 1;

	bool quiet = false;
	bool use_E_base_isa = false;

	TinyOptions(void) {
		// clang-format off
		add_options()
			("quiet", po::bool_switch(&quiet), "do not output register values on exit")
			("memory-start", po::value<unsigned int>(&mem_start_addr), "set memory start address")
			("memory-size", po::value<unsigned int>(&mem_size), "set memory size")
			("use-E-base-isa", po::bool_switch(&use_E_base_isa), "use the E instead of the I integer base ISA");
        	// clang-format on
        }

	void parse(int argc, char **argv) override {
		Options::parse(argc, argv);
		mem_end_addr = mem_start_addr + mem_size - 1;
	}
};

int sc_main(int argc, char **argv) {
	TinyOptions opt;
	opt.parse(argc, argv);

	std::srand(std::time(nullptr));  // use current time as seed for random generator

	tlm::tlm_global_quantum::instance().set(sc_core::sc_time(opt.tlm_global_quantum, sc_core::SC_NS));

	ISS core0(0);
	ISS core1(1);
	ISS core2(2);
	ISS core3(3);
	CombinedMemoryInterface core0_mem_if("MemoryInterface0", core0);
	CombinedMemoryInterface core1_mem_if("MemoryInterface1", core1); 
	CombinedMemoryInterface core2_mem_if("MemoryInterface2", core2);
	CombinedMemoryInterface core3_mem_if("MemoryInterface3", core3); 

	SimpleMemory mem("SimpleMemory", opt.mem_size);
	ELFLoader loader(opt.input_program.c_str());
	// SimpleBus<3, 3> bus("SimpleBus");
	SimpleBus<9, 12> bus("SimpleBus");
	SyscallHandler sys("SyscallHandler");
	FE310_PLIC<1, 64, 96, 32> plic("PLIC");
	CLINT<4> clint("CLINT");
	SimpleDMA dma0("SimpleDMA0", 4);
	SimpleDMA dma1("SimpleDMA1", 4);
	SimpleDMA dma2("SimpleDMA2", 4);
	SimpleDMA dma3("SimpleDMA3", 4);
	DebugMemoryInterface dbg_if("DebugMemoryInterface");
	Sorter sorter0("sorter0");
	Sorter sorter1("sorter1");
	Sorter sorter2("sorter2");
	Sorter sorter3("sorter3");


	std::shared_ptr<BusLock> bus_lock = std::make_shared<BusLock>();
	core0_mem_if.bus_lock = bus_lock;
	core1_mem_if.bus_lock = bus_lock;
	core2_mem_if.bus_lock = bus_lock;
	core3_mem_if.bus_lock = bus_lock;

	bus.ports[0] = new PortMapping(opt.mem_start_addr, opt.mem_end_addr);
	bus.ports[1] = new PortMapping(opt.clint_start_addr, opt.clint_end_addr);
	bus.ports[2] = new PortMapping(opt.sys_start_addr, opt.sys_end_addr);
	bus.ports[3] = new PortMapping(opt.sorter_start_addr0, opt.sorter_end_addr0);
	bus.ports[4] = new PortMapping(opt.sorter_start_addr1, opt.sorter_end_addr1);
	bus.ports[5] = new PortMapping(opt.sorter_start_addr2, opt.sorter_end_addr2);
	bus.ports[6] = new PortMapping(opt.sorter_start_addr3, opt.sorter_end_addr3);
	bus.ports[7] = new PortMapping(opt.dma0_start_addr, opt.dma0_end_addr);	
	bus.ports[8] = new PortMapping(opt.dma1_start_addr, opt.dma1_end_addr);
	bus.ports[9] = new PortMapping(opt.dma2_start_addr, opt.dma2_end_addr);
	bus.ports[10] = new PortMapping(opt.dma3_start_addr, opt.dma3_end_addr);
	bus.ports[11] = new PortMapping(opt.plic_start_addr, opt.plic_end_addr);

	loader.load_executable_image(mem, mem.size, opt.mem_start_addr);

	core0.init(&core0_mem_if, &core0_mem_if, &clint, loader.get_entrypoint(),
	           opt.mem_end_addr - 3);  // -3 to not overlap with the next region and stay 32 bit aligned
	core1.init(&core1_mem_if, &core1_mem_if, &clint, loader.get_entrypoint(), opt.mem_end_addr - 32767);
	core2.init(&core2_mem_if, &core2_mem_if, &clint, loader.get_entrypoint(), opt.mem_end_addr - 65535);
	core3.init(&core3_mem_if, &core3_mem_if, &clint, loader.get_entrypoint(), opt.mem_end_addr - 98303);

	sys.init(mem.data, opt.mem_start_addr, loader.get_heap_addr());
	sys.register_core(&core0);
	sys.register_core(&core1);
	sys.register_core(&core2);
	sys.register_core(&core3);

	if (opt.intercept_syscalls) {
		core0.sys = &sys;
		core1.sys = &sys;
		core2.sys = &sys;
		core3.sys = &sys;
	}

	// connect TLM sockets
	core0_mem_if.isock.bind(bus.tsocks[0]);
	core1_mem_if.isock.bind(bus.tsocks[1]);
	core2_mem_if.isock.bind(bus.tsocks[2]);
	core3_mem_if.isock.bind(bus.tsocks[3]);
	dbg_if.isock.bind(bus.tsocks[8]);

	PeripheralWriteConnector dma0_connector("SimpleDMA-Connector");  // to respect ISS bus locking
	PeripheralWriteConnector dma1_connector("SimpleDMA-Connector");
	PeripheralWriteConnector dma2_connector("SimpleDMA-Connector");
	PeripheralWriteConnector dma3_connector("SimpleDMA-Connector");
	dma0_connector.isock.bind(bus.tsocks[4]);
	dma1_connector.isock.bind(bus.tsocks[5]);
	dma2_connector.isock.bind(bus.tsocks[6]);
	dma3_connector.isock.bind(bus.tsocks[7]);
	dma0.isock.bind(dma0_connector.tsock);
	dma1.isock.bind(dma1_connector.tsock);
	dma2.isock.bind(dma2_connector.tsock);
	dma3.isock.bind(dma3_connector.tsock);
	dma0_connector.bus_lock = bus_lock;
	dma1_connector.bus_lock = bus_lock;
	dma2_connector.bus_lock = bus_lock;
	dma3_connector.bus_lock = bus_lock;

	bus.isocks[0].bind(mem.tsock);
	bus.isocks[1].bind(clint.tsock);
	bus.isocks[2].bind(sys.tsock);
	bus.isocks[3].bind(sorter0.tsock);
	bus.isocks[4].bind(sorter1.tsock);
	bus.isocks[5].bind(sorter2.tsock);
	bus.isocks[6].bind(sorter3.tsock);
	bus.isocks[7].bind(dma0.tsock);
	bus.isocks[8].bind(dma1.tsock);
	bus.isocks[9].bind(dma2.tsock);
	bus.isocks[10].bind(dma3.tsock);
	bus.isocks[11].bind(plic.tsock);

	// connect interrupt signals/communication
	plic.target_harts[0] = &core0;
	plic.target_harts[1] = &core1;
	plic.target_harts[2] = &core2;
	plic.target_harts[3] = &core3;
	clint.target_harts[0] = &core0;
	clint.target_harts[1] = &core1;
	clint.target_harts[2] = &core2;
	clint.target_harts[3] = &core3;
	dma0.plic = &plic;
	dma1.plic = &plic;
	dma2.plic = &plic;
	dma3.plic = &plic;

	// switch for printing instructions
	core0.trace = opt.trace_mode;
	core1.trace = opt.trace_mode;
	core2.trace = opt.trace_mode;
	core3.trace = opt.trace_mode;

	std::vector<debug_target_if *> threads;
	threads.push_back(&core0);
	threads.push_back(&core1);
	threads.push_back(&core2);
	threads.push_back(&core3);

	if (opt.use_debug_runner) {
		auto server = new GDBServer("GDBServer", threads, &dbg_if, opt.debug_port);
		new GDBServerRunner("GDBRunner0", server, &core0);
		new GDBServerRunner("GDBRunner1", server, &core1);
		new GDBServerRunner("GDBRunner2", server, &core2);
		new GDBServerRunner("GDBRunner3", server, &core3);
	} else {
		new DirectCoreRunner(core0);
		new DirectCoreRunner(core1);
		new DirectCoreRunner(core2);
		new DirectCoreRunner(core3);
	}

	if (opt.quiet)
		sc_core::sc_report_handler::set_verbosity_level(sc_core::SC_NONE);

	sc_core::sc_start();
	if (!opt.quiet) {
		core0.show();
		core1.show();
		core2.show();
		core3.show();
	}

	return 0;
}