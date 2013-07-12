#pragma once

#ifdef eMU_UT
#include <ut/env/asioStub/ioService.hpp>
#include <ut/env/asioStub/tcp/acceptor.hpp>
#elif eMU_MT
#include <mt/env/asioStub/ioService.hpp>
#include <mt/env/asioStub/tcp/acceptor.hpp>
#else
#include <boost/asio.hpp>
#endif

#include <boost/noncopyable.hpp>
#include <functional>

#include <network/buffer.hpp>
#include <network/tcp/connection.hpp>


namespace eMU {
namespace core {
namespace network {
namespace tcp {

#ifdef eMU_UT
namespace asio = eMU::ut::env::asioStub;
#elif eMU_MT
namespace asio = eMU::mt::env::asioStub;
#else
namespace asio = boost::asio;
#endif

class ConnectionsManager: boost::noncopyable {
public:
    typedef std::function<size_t()> GenerateConnectionHashCallback;
    typedef std::function<void(size_t, const Payload&)> ReceiveEventCallback;
    typedef std::function<void(size_t)> CloseEventCallback;
    typedef std::function<void(size_t)> AcceptEventCallback;

    ConnectionsManager(asio::io_service &ioService, int16_t port);
    virtual ~ConnectionsManager();

    void queueAccept();

    void setGenerateConnectionHashCallback(const GenerateConnectionHashCallback &callback);
    void setAcceptEventCallback(const AcceptEventCallback &callback);
    void setReceiveEventCallback(const ReceiveEventCallback &callback);
    void setCloseEventCallback(const CloseEventCallback &callback);

    void send(size_t hash, const Payload &payload);
    void disconnect(size_t hash);

    #ifdef eMU_UT
    asio::ip::tcp::acceptor& acceptor();
    #endif

private:
    void acceptHandler(Connection::Pointer connection, const boost::system::error_code &errorCode);
    void registerConnection(Connection::Pointer connection);
    void receiveEvent(Connection &connection);
    void closeEvent(Connection &connection);
    size_t findConnectionHash(const Connection &connection) const;

    asio::io_service ioService_;
    asio::ip::tcp::acceptor acceptor_;
    std::map<size_t, Connection::Pointer> connections_;

    GenerateConnectionHashCallback generateConnectionHashCallback_;
    AcceptEventCallback acceptEventCallback_;
    ReceiveEventCallback receiveEventCallback_;
    CloseEventCallback closeEventCallback_;
};

}
}
}
}