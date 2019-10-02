#include <marlin/lpf/LpfTransportFactory.hpp>
#include <spdlog/spdlog.h>

#include <marlin/net/tcp/TcpTransportFactory.hpp>

using namespace marlin::net;
using namespace marlin::lpf;

template<typename Delegate>
using TransportType = LpfTransport<Delegate, TcpTransport, 8>;

template<typename ListenDelegate, typename TransportDelegate>
using TransportFactoryType = LpfTransportFactory<ListenDelegate, TransportDelegate, TcpTransportFactory, TcpTransport, 8>;

struct Delegate {
	void did_recv_message(TransportType<Delegate> &transport, Buffer &&message) {
		SPDLOG_INFO(
			"Transport {{ Src: {}, Dst: {} }}: Did recv message: {} bytes",
			transport.src_addr.to_string(),
			transport.dst_addr.to_string(),
			message.size()
		);
	}

	void did_send_message(TransportType<Delegate> &transport, Buffer &&message) {
		SPDLOG_INFO(
			"Transport {{ Src: {}, Dst: {} }}: Did send message: {} bytes",
			transport.src_addr.to_string(),
			transport.dst_addr.to_string(),
			message.size()
		);
		transport.close();
	}

	void did_dial(TransportType<Delegate> &transport) {
		transport.send(Buffer(new char[10], 10));
	}

	bool should_accept(SocketAddress const &) {
		return true;
	}

	void did_create_transport(TransportType<Delegate> &transport) {
		transport.setup(this);
	}

	void did_close(TransportType<Delegate> &) {
		SPDLOG_INFO("Did close");
	}
};

int main() {
	TransportFactoryType<Delegate, Delegate> s, c;
	s.bind(SocketAddress::loopback_ipv4(8000));

	Delegate d;

	s.listen(d);

	c.bind(SocketAddress::loopback_ipv4(0));
	c.dial(SocketAddress::loopback_ipv4(8000), d);

	c.get_transport(SocketAddress::loopback_ipv4(1234));

	return uv_run(uv_default_loop(), UV_RUN_DEFAULT);
}
