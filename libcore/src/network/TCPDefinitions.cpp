#include "util/Standard.hh"
#include "TCPDefinitions.hpp"
#include "IOService.hpp"

namespace Sirikata {
namespace Network {

TCPSocket::TCPSocket(IOService&io):
    boost::asio::ip::tcp::socket(*(io.mImpl)){}


TCPListener::TCPListener(IOService&io, const boost::asio::ip::tcp::endpoint&ep):
    boost::asio::ip::tcp::acceptor(*(io.mImpl),ep){}

void TCPListener::async_accept(TCPSocket&socket,
                               const std::tr1::function<void(const boost::system::error_code&)>&cb) {
    this->InternalTCPAcceptor::async_accept(socket,cb);
}


TCPResolver::TCPResolver(IOService&io)
    : boost::asio::ip::tcp::resolver(*(io.mImpl))
{
}

} // namespace Network
} // namespace Sirikata
