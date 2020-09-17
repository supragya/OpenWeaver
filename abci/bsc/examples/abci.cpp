#include "marlin/bsc/Abci.hpp"

#include <marlin/asyncio/core/EventLoop.hpp>

#include <structopt/app.hpp>

using namespace marlin::bsc;
using namespace marlin::asyncio;
using namespace marlin::core;

struct Delegate {
	template<typename AbciType>
	void did_connect(AbciType& abci) {
		abci.get_block_number();
	}

	template<typename AbciType>
	void did_disconnect(AbciType&) {
		SPDLOG_INFO("Did disconnect");
	}

	template<typename AbciType>
	void did_analyze_block(
		AbciType&,
		Buffer&&,
		std::string hash,
		std::string coinbase,
		WeakBuffer header
	) {
		SPDLOG_INFO(
			"Analyzed block: Hash: {}, Coinbase: {}, Header: {}",
			hash,
			coinbase,
			spdlog::to_hex(header.data(), header.data() + header.size())
		);
	}

	template<typename AbciType>
	void did_close(AbciType&) {
		SPDLOG_INFO("Did close");
	}
};

struct Options {
	std::string datadir;
};
STRUCTOPT(Options, datadir);

int main(int argc, char** argv) {
	try {
		auto options = structopt::app("abci").parse<Options>(argc, argv);

		Abci<Delegate> abci(options.datadir);

		return EventLoop::run();
	} catch (structopt::exception& e) {
		SPDLOG_ERROR("{}", e.what());
		SPDLOG_ERROR("{}", e.help());
	}

	return -1;
}
