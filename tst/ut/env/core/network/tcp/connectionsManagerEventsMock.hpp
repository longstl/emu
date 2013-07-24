#pragma once

#include <gmock/gmock.h>
#include <core/network/buffer.hpp>

namespace eMU
{
namespace ut
{
namespace env
{
namespace core
{
namespace tcp
{

class ConnectionsManagerEventsMock
{
public:
    MOCK_METHOD0(generateConnectionHash, size_t());
    MOCK_METHOD1(acceptEvent, void(size_t));
    MOCK_METHOD2(receiveEvent, void(size_t, const eMU::core::network::Payload&));
    MOCK_METHOD1(closeEvent, void(size_t));
};

}
}
}
}
}
