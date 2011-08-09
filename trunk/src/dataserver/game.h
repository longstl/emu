#ifndef eMU_DATASERVER_GAME_H
#define eMU_DATASERVER_GAME_H

#include "protocol.h"
#include "user.h"
#include "configuration.h"
#include "database.h"

#pragma warning(disable: 4355)

class game_t: public protocolExecutorInterface_t {
public:
    typedef boost::function1<void, dataServerUser_t*> disconnectCallback_t;

    game_t(const disconnectCallback_t &disconnectCallback);

    void startup();
    void cleanup();

    // --------------------------------------------------------
    // Interface for protocolExecutorInterface_t.
    void onAccountCheckRequest(dataServerUser_t *user,
                               unsigned int connectionStamp,
                               const std::string &accountId,
                               const std::string &password,
                               const std::string &ipAddress);

    void onCharacterListRequest(dataServerUser_t *user,
                                unsigned int connectionStamp,
                                const std::string &accountId);

    void onLogoutRequest(dataServerUser_t *user,
                         const std::string &accountId);

    void onCharacterCreateRequest(dataServerUser_t *user,
                                  unsigned int connectionStamp,
                                  const std::string &accountId,
                                  const std::string &name,
                                  eMUShared::characterRace_e::type_t race);

    void onCharacterDeleteRequest(dataServerUser_t *user,
                                  unsigned int connectionStamp,
                                  const std::string &accountId,
                                  const std::string &name,
                                  const std::string &pin);

    void onCharacterSelectRequest(dataServerUser_t *user,
                                  unsigned int connectionStamp,
                                  const std::string &accountId,
                                  const std::string &name);

    void onCharacterSaveRequest(dataServerUser_t *user,
                                const std::string &accountId,
                                const eMUShared::characterAttributes_t &attr);
    // --------------------------------------------------------

    void onQueryExceptionNotice(dataServerUser_t *user,
                                unsigned int connectionStamp,
                                const std::string &what);

    void protocol(protocol_t *protocol) {
        m_protocol = protocol;
    }
    void logger(eMUCore::logger_t *logger) {
        m_logger = logger;
    }
    void scheduler(eMUCore::scheduler_t *scheduler) {
        m_scheduler = scheduler;
    }

private:
    game_t();
    game_t(const game_t &game);
    game_t& operator=(const game_t &game);

    configuration_t			m_configuration;
    database_t				m_database;
    disconnectCallback_t	m_disconnectCallback;
    protocol_t				*m_protocol;
    eMUCore::logger_t		*m_logger;
    eMUCore::scheduler_t	*m_scheduler;
};

#pragma warning(default: 4355)

#endif // eMU_DATASERVER_GAME_H