#include <loginserver/transactions/loginRequest.hpp>
#include <loginserver/user.hpp>

#include <protocol/readStream.hpp>
#include <protocol/dataserver/checkAccountRequest.hpp>
#include <protocol/dataserver/messageIds.hpp>
#include <protocol/loginserver/loginRequest.hpp>

#include <ut/env/core/network/tcp/connectionMock.hpp>

#include <gtest/gtest.h>
#include <boost/locale.hpp>

using ::testing::_;
using ::testing::Return;
using ::testing::SaveArg;

using eMU::ut::env::core::network::tcp::ConnectionMock;
using eMU::core::network::Payload;
using eMU::protocol::ReadStream;
using eMU::protocol::loginserver::LoginRequest;
using eMU::loginserver::User;
using eMU::protocol::dataserver::CheckAccountRequest;
namespace MessageIds = eMU::protocol::dataserver::MessageIds;

class LoginRequestTransactionTest: public ::testing::Test
{
protected:
    LoginRequestTransactionTest():
        accountId_(L"testAccount"),
        password_(L"testPassword"),
        request_(ReadStream(LoginRequest(accountId_, password_).getWriteStream().getPayload())),
        dataserverConnection_(new ConnectionMock()),
        connection_(new ConnectionMock()),
        user_(connection_) {}

    std::wstring accountId_;
    std::wstring password_;
    LoginRequest request_;
    ConnectionMock::Pointer dataserverConnection_;
    ConnectionMock::Pointer connection_;
    User user_;
};

TEST_F(LoginRequestTransactionTest, handle)
{
    Payload payload;
    EXPECT_CALL(*dataserverConnection_, send(_)).WillOnce(SaveArg<0>(&payload));
    EXPECT_CALL(*dataserverConnection_, isOpen()).WillOnce((Return(true)));

    eMU::loginserver::transactions::LoginRequest(user_, dataserverConnection_, request_).handle();

    ReadStream readStream(payload);
    ASSERT_EQ(MessageIds::kCheckAccountRequest, readStream.getId());

    CheckAccountRequest checkAccountRequest(readStream);
    ASSERT_EQ(user_.getHash(), checkAccountRequest.getClientHash());
    ASSERT_EQ(boost::locale::conv::utf_to_utf<std::string::value_type>(accountId_), checkAccountRequest.getAccountId());
    ASSERT_EQ(boost::locale::conv::utf_to_utf<std::string::value_type>(password_), checkAccountRequest.getPassword());
    ASSERT_EQ(boost::locale::conv::utf_to_utf<std::string::value_type>(accountId_), user_.getAccountId());
}

TEST_F(LoginRequestTransactionTest, WhenConnectionToDataserverIsNotOpenThenClientShouldBeDisconnected)
{
    EXPECT_CALL(*dataserverConnection_, isOpen()).WillOnce((Return(false)));
    EXPECT_CALL(*connection_, disconnect());

    eMU::loginserver::transactions::LoginRequest(user_, dataserverConnection_, request_).handle();
}

TEST_F(LoginRequestTransactionTest, WhenNullPtrAsDataserverConnectionProvidedThenClientShouldBeDisconnected)
{
    EXPECT_CALL(*connection_, disconnect());

    eMU::loginserver::transactions::LoginRequest(user_, nullptr, request_).handle();
}
