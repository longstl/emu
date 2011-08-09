// -----------------------------------------------
//
// Solution name:			eMU
// Project name:			core
// File name:				core.h
// Created:					2010-05-09 18:32
// Author:					f1x
//
// -----------------------------------------------
#ifndef eMU_CORE_CORE_H
#define eMU_CORE_CORE_H

#include <winsock2.h>
#include <windows.h>
#include <libxml/parser.h>
#include <boost/lexical_cast.hpp>
#include <boost/function.hpp>
#include <string>
#include <sstream>
#include <vector>
#include <fstream>
#include <map>
#include <iomanip>
#include "enum.h"

#pragma warning(disable: 4251)
#pragma warning(disable: 4290)

#ifdef eMUCORE_EXPORTS
#define eMUCORE_DECLSPEC __declspec(dllexport)
#else
#define eMUCORE_DECLSPEC __declspec(dllimport)
#endif

namespace eMUCore {

template<typename T>
T role(const T &min, const T &max) {
    T cpMax = max;

    if(cpMax >= min) {
        cpMax -= min;
    } else {
        T tempRange = min - max;
        min = max;
        max = tempRange;
    }

    return static_cast<T>(cpMax ? min + rand() / (RAND_MAX + 1.0) * static_cast<double>(cpMax) : min);
}

eMUCORE_DECLSPEC bool isIpAddress(const std::string &hostname);
eMUCORE_DECLSPEC std::string convertToIpAddress(const std::string &hostname);

class eMUCORE_DECLSPEC exception_t: public std::exception {
public:
    exception_t() {}

    exception_t(const exception_t &e) {
        m_stream << e.m_stream.rdbuf();
    }

    virtual ~exception_t() {}

    inline std::stringstream& in() {
        m_stream.str("");
        return m_stream;
    }

    const char* what() {
        m_message = m_stream.str();
        return m_message.c_str();
    }

private:
    std::stringstream m_stream;
    std::string m_message;
};

class eMUCORE_DECLSPEC synchronizer_t {
public:
    synchronizer_t() {
        InitializeCriticalSection(&m_criticalSection);
    }
    ~synchronizer_t() {
        DeleteCriticalSection(&m_criticalSection);
    }
    inline void lock() {
        EnterCriticalSection(&m_criticalSection);
    }
    inline void unlock() {
        LeaveCriticalSection(&m_criticalSection);
    }

private:
    synchronizer_t(const synchronizer_t&);
    synchronizer_t& operator=(const synchronizer_t&);

    CRITICAL_SECTION m_criticalSection;
};

class eMUCORE_DECLSPEC profiler_t {
public:
    profiler_t(const std::string &functionName):
        m_functionName(functionName) {
        QueryPerformanceCounter(&m_startTime);
    }

    ~profiler_t() {
        time_t duration = this->duration();

        static synchronizer_t synchronizer;
        synchronizer.lock();

        static std::ofstream file("log\\profiles.txt", std::ios::out | std::ios::app);

        if(file) {
            file << m_functionName << "() duration: " << duration / 1000 << "ms." << std::endl;
        }

        synchronizer.unlock();
    }

private:
    profiler_t();
    profiler_t(const profiler_t&);
    profiler_t& operator=(const profiler_t&);

    time_t duration() {
        LARGE_INTEGER endTime;
        QueryPerformanceCounter(&endTime);

        LARGE_INTEGER frequency;
        QueryPerformanceFrequency(&frequency);

        endTime.QuadPart -= m_startTime.QuadPart;
        endTime.QuadPart *= 1000000;
        endTime.QuadPart /= frequency.QuadPart;

        if(endTime.HighPart != 0) {
            return -1;
        } else {
            return endTime.LowPart;
        }
    }

    std::string		m_functionName;
    LARGE_INTEGER	m_startTime;
};

#ifdef _PROFILING
#define _PROFILE eMUCore::profiler_t profiler(__FUNCTION__)
#else
#define _PROFILE
#endif

class eMUCORE_DECLSPEC logger_t {
public:
    logger_t::logger_t();

    void startup() throw(eMUCore::exception_t);
    void cleanup();
    std::stringstream& in(loggerMessage_e::type_t type) throw(eMUCore::exception_t);
    inline std::stringstream& append() {
        return m_stream;
    }
    void out();

private:
    logger_t(const logger_t&);
    logger_t& operator=(const logger_t&);

    inline void color(unsigned short color) {
        SetConsoleTextAttribute(m_stdOutput, color);
    }
    void openFile() throw(eMUCore::exception_t);

    std::stringstream		m_date;
    std::stringstream		m_stream;
    synchronizer_t			m_synchronizer;
    loggerMessage_e::type_t	m_currentMessageType;
    std::ofstream			m_file;
    HANDLE					m_stdOutput;
    SYSTEMTIME				m_openFileTime;

    static const std::string c_loggerMessageHeader[];
    static const unsigned char c_loggerMessageColor[];
};

const size_t c_ioDataMaxSize = 8192;

struct eMUCORE_DECLSPEC ioBuffer_t {
public:
    ioBuffer_t();
    virtual ~ioBuffer_t() {}

    inline void clearData() {
        memset(m_data, 0, c_ioDataMaxSize);
        m_dataSize = 0;
    }

    WSAOVERLAPPED	m_wsaOverlapped;
    WSABUF			m_wsaBuff;
    unsigned char	m_data[c_ioDataMaxSize];
    size_t			m_dataSize;
    ioBuffer_e::type_t m_type;

private:
    ioBuffer_t(const ioBuffer_t&);
    ioBuffer_t& operator=(const ioBuffer_t&);
};

struct eMUCORE_DECLSPEC ioSendBuffer_t: public ioBuffer_t {
public:
    ioSendBuffer_t();

    inline void clearSecondData() {
        memset(m_secondData, 0, c_ioDataMaxSize);
        m_secondDataSize = 0;
    }

    inline void moveSecondData() {
        memcpy(m_data, m_secondData, m_secondDataSize);
        m_dataSize = m_secondDataSize;
        m_wsaBuff.len = m_secondDataSize;
    }

    inline void mergeSecondData(const unsigned char *data, size_t dataSize) {
        memcpy(&m_secondData[m_secondDataSize], data, dataSize);
        m_secondDataSize += dataSize;
    }

    bool			m_dataLocked;
    unsigned char	m_secondData[c_ioDataMaxSize];
    size_t			m_secondDataSize;

private:
    ioSendBuffer_t(const ioSendBuffer_t&);
    ioSendBuffer_t& operator=(const ioSendBuffer_t&);
};

class eMUCORE_DECLSPEC socketContext_t {
public:
    typedef boost::function1<void, socketContext_t*> callback_t;

    socketContext_t(int index);
    virtual ~socketContext_t() {}

    void preClose();
    void postClose();

    friend std::ostream& operator<<(std::ostream &out, const socketContext_t &context) {
        out << "userId: " << context.m_index << ", address: " << context.m_ipAddress;
        return out;
    }

    inline void activate() {
        m_active = true;
    }
    inline void deactivate() {
        m_active = false;
    }
    inline bool active() const {
        return m_active;
    }

    inline void socket(SOCKET socket) {
        m_socket = socket;
    }
    inline SOCKET socket() const {
        return m_socket;
    }

    inline ioBuffer_t& recvBuffer() {
        return m_recvBuffer;
    }
    inline ioSendBuffer_t& sendBuffer() {
        return m_sendBuffer;
    }

    inline void ipAddress(const std::string &ipAddress) {
        m_ipAddress = ipAddress;
    }
    inline const std::string& ipAddress() const {
        return m_ipAddress;
    }

    inline void port(unsigned short port) {
        m_port = port;
    }
    inline unsigned short port() const {
        return m_port;
    }

    inline int index() const {
        return m_index;
    }

    inline void callbacks(const socketContext_t::callback_t &onAttach,
                          const socketContext_t::callback_t &onReceive,
                          const socketContext_t::callback_t &onClose) {
        m_onAttach = onAttach;
        m_onReceive = onReceive;
        m_onClose = onClose;
    }

    inline void onAttach() {
        m_onAttach(this);
    }
    inline void onReceive() {
        m_onReceive(this);
    }
    inline void onClose() {
        m_onClose(this);
    }

    virtual bool operator==(const int &key) {
        return key == m_index;
    }

protected:
    std::string			m_ipAddress;
    ioBuffer_t			m_recvBuffer;
    ioSendBuffer_t		m_sendBuffer;
    bool				m_active;
    SOCKET				m_socket;
    unsigned short		m_port;
    int					m_index;
    callback_t			m_onAttach;
    callback_t			m_onReceive;
    callback_t			m_onClose;

private:
    socketContext_t();
    socketContext_t(const socketContext_t&);
    socketContext_t& operator=(const socketContext_t&);
};

template<typename T>
class socketContextManager_t {
public:
    socketContextManager_t() {}

    void startup(size_t count,
                 const socketContext_t::callback_t &onAttach,
                 const socketContext_t::callback_t &onReceive,
                 const socketContext_t::callback_t &onClose) {
        for(size_t i = 0; i < count; ++i) {
            T *ctx = new T(i);
            ctx->callbacks(onAttach, onReceive, onClose);

            m_list.push_back(ctx);
        }
    }

    void cleanup() {
        for(size_t i = 0; i < m_list.size(); ++i) {
            delete m_list[i];
        }

        m_list.clear();
    }

    inline size_t size() const {
        return m_list.size();
    }
    T* operator[](size_t index) {
        return m_list[index];
    }

    template<typename key_t>
    T* find(const key_t& key) {
        for(size_t i = 0; i < m_list.size(); ++i) {
            if((*m_list[i]) == key) {
                return *m_list[i];
            }
        }

        eMUCore::exception_t e;
        e.in() << "Couldn't found object by key: " << key;
        throw e;
    }

    T* findFree() {
        for(size_t i = 0; i < m_list.size(); ++i) {
            if(!m_list[i]->active()) {
                return m_list[i];
            }
        }

        return NULL;
    }

private:
    socketContextManager_t(const socketContextManager_t&);
    socketContextManager_t& operator=(const socketContextManager_t&);

    std::vector<T*> m_list;
};

class eMUCORE_DECLSPEC iocpEngine_t {
public:
    iocpEngine_t();

    void startup() throw(eMUCore::exception_t);
    void cleanup();
    void attach(socketContext_t *context) const;
    void detach(socketContext_t *context) const;
    void write(socketContext_t *context, const unsigned char *data, size_t dataSize) const;

    void logger(logger_t *logger) {
        m_logger = logger;
    }
    void synchronizer(synchronizer_t *synchronizer) {
        m_synchronizer = synchronizer;
    }

private:
    iocpEngine_t(const iocpEngine_t&);
    iocpEngine_t& operator=(const iocpEngine_t&);

    void dequeueReceive(socketContext_t *context) const;
    void dequeueSend(socketContext_t *context) const;
    void dequeueError(socketContext_t *context, int lastError) const;

    static DWORD worker(iocpEngine_t *instance);

    logger_t						*m_logger;
    synchronizer_t					*m_synchronizer;
    PHANDLE							m_workerThread;
    HANDLE							m_ioCompletionPort;
    size_t							m_workerThreadCount;
};

class eMUCORE_DECLSPEC tcpServer_t {
public:
    typedef boost::function0<socketContext_t*> callback_t;

    tcpServer_t(const callback_t &onAllocate);

    void startup(unsigned short listenPort) throw (eMUCore::exception_t);
    void cleanup();

    inline unsigned short listenPort() const {
        return m_listenPort;
    }

    void logger(logger_t *logger) {
        m_logger = logger;
    }
    void iocpEngine(iocpEngine_t *iocpEngine) {
        m_iocpEngine = iocpEngine;
    }

private:
    tcpServer_t();
    tcpServer_t(const tcpServer_t&);
    tcpServer_t& operator=(const tcpServer_t&);

    static DWORD worker(tcpServer_t *instance);

    logger_t						*m_logger;
    iocpEngine_t					*m_iocpEngine;
    SOCKET							m_socket;
    HANDLE							m_acceptWorkerThread;
    unsigned short					m_listenPort;
    callback_t						m_onAllocate;
};

class eMUCORE_DECLSPEC tcpClient_t: public socketContext_t {
public:
    tcpClient_t();

    bool connect(const std::string &address, unsigned short port) throw(eMUCore::exception_t);
    inline void disconnect() {
        m_iocpEngine->detach(this);
    }

    void logger(logger_t *logger) {
        m_logger = logger;
    }
    void iocpEngine(iocpEngine_t *iocpEngine) {
        m_iocpEngine = iocpEngine;
    }

private:
    tcpClient_t(const tcpClient_t&);
    tcpClient_t& operator=(const tcpClient_t&);

    logger_t		*m_logger;
    iocpEngine_t	*m_iocpEngine;
};

class eMUCORE_DECLSPEC udpSocket_t {
public:
    typedef boost::function3<void, const sockaddr_in&, unsigned char*, size_t> callback_t;

    udpSocket_t(const callback_t &onReceive);

    void startup(unsigned short port) throw(eMUCore::exception_t);
    void cleanup();
    void worker() const;
    void send(std::string hostname,
              unsigned short port,
              const unsigned char *buffer,
              size_t bufferSize) const;

    unsigned short port() const {
        return m_port;
    }
    void logger(logger_t *logger) {
        m_logger = logger;
    }

private:
    udpSocket_t();
    udpSocket_t(const udpSocket_t&);
    udpSocket_t& operator=(const udpSocket_t&);

    logger_t						*m_logger;
    SOCKET							m_socket;
    unsigned short					m_port;
    callback_t						m_onReceive;
};

class eMUCORE_DECLSPEC xmlConfig_t {
public:
    xmlConfig_t();

    void open(const std::string &fileName, const std::string &rootNodeName, bool autoBegin = true) throw(eMUCore::exception_t);
    void close();

    template <typename T>
    T readFromNode(const std::string &nodeName, const T &defaultValue) {
        if(m_node != NULL) {
            while(m_node) {
                if(xmlStrcmp(m_node->name, reinterpret_cast<const xmlChar*>(nodeName.c_str())) == 0) {
                    xmlChar *readValue = xmlNodeListGetString(m_configFile, m_node->children, 1);

                    if(readValue != NULL) {
                        try {
                            return boost::lexical_cast<T>(std::string(reinterpret_cast<char*>(readValue)));
                        } catch(std::exception &lce) {
                            eMUCore::exception_t e;
                            e.in() << "[xmlConfig_t::readFromNode()] " << lce.what() << " :: node: " << nodeName
                                   << ", value: " << readValue << ".";
                            throw e;
                        }
                    } else {
                        break;
                    }
                } else {
                    m_node = m_node->next;
                    continue;
                }
            }
        }

        return defaultValue;
    }

    template <typename T>
    T readFromProperty(const std::string &nodeName, const std::string &propertyName, const T &defaultValue) const {
        if(xmlStrcmp(m_node->name, reinterpret_cast<const xmlChar*>(nodeName.c_str())) == 0) {
            xmlChar *readValue = xmlGetProp(m_node, reinterpret_cast<const xmlChar*>(propertyName.c_str()));

            if(readValue != NULL) {
                try {
                    return boost::lexical_cast<T>(std::string(reinterpret_cast<char*>(readValue)));
                } catch(std::exception &lce) {
                    eMUCore::exception_t e;
                    e.in() << "[xmlConfig_t::readFromProperty()] " << lce.what() << " :: property: " << propertyName
                           << " value: " << readValue << ".";
                    throw e;
                }
            }
        } else {
            eMUCore::exception_t e;
            e.in() << "[xmlConfig_t::readFromProperty()] Current node: " << reinterpret_cast<const char*>(m_node->name)
                   << ", expected node: " << nodeName << "].";
            throw e;
        }

        return defaultValue;
    }

    bool readBoolFromProperty(const std::string &nodeName, const std::string &propertyName, bool defaultValue);

    bool nextNode();
    bool childrenNode();
    bool parentNode();

private:
    xmlConfig_t(const xmlConfig_t&);
    xmlConfig_t& operator=(const xmlConfig_t&);

    xmlDocPtr	m_configFile;
    xmlNodePtr	m_rootNode;
    xmlNodePtr	m_node;
    bool		m_firstIteration;
    std::string m_rootNodeName;
};

class eMUCORE_DECLSPEC packet_t {
public:
    packet_t();
    packet_t(const unsigned char *data);

    friend std::ostream& operator<<(std::ostream &out, const packet_t &packet) {
        out << "data: ";

        for(size_t i = 0; i < packet.m_size; ++i) {
            out << std::setfill('0')
                << std::setw(2)
                << std::hex
                << std::uppercase
                << static_cast<int>(packet.m_data[i]);

            if(i < packet.m_size - 1) {
                out << " ";
            }
        }

        out << std::dec;
        return out;
    }

    void construct(unsigned char headerId, unsigned char protocolId);
    void cryptSerial(unsigned char cryptSerial);
    void clear();

    template<typename T>
    void insert(size_t offset, const T &value) {
        size_t valueSize = sizeof(T);

        if(offset + valueSize <= m_maxSize) {
            memcpy(&m_data[offset], &value, valueSize);
            this->increaseSize(valueSize);
        } else {
            exception_t e;
            e.in() << __FILE__ << ":" << __LINE__ << " [packet_t::insert()] value: " << value
                   << ", offset: " << offset << " is out of range.";
            throw e;
        }
    }

    void insertString(size_t offset, const std::string &value, size_t totalSize);

    template<typename T>
    T read(size_t offset) const {
        size_t valueSize = sizeof(T);

        if(offset + valueSize <= m_size) {
            T value;
            memcpy(&value, &m_data[offset], sizeof(T));
            return value;
        } else {
            exception_t e;
            e.in() << __FILE__ << ":" << __LINE__ << " [packet_t::read()] valueSize: " << valueSize
                   << ", offset: " << offset << ", size: " << m_size << " is out of range.";
            throw e;
        }
    }

    std::string readString(size_t offset, size_t size) const;

    inline unsigned char headerId() const {
        return m_headerId;
    }
    inline size_t size() const {
        return m_size;
    }
    inline unsigned char protocolId() const {
        return m_protocolId;
    }
    inline const unsigned char* data() const {
        return m_data;
    }
    inline size_t headerSize() const {
        return m_headerSize;
    }
    inline bool crypted() const {
        return rawDataCrypted(m_data);
    }

    static size_t rawDataSize(const unsigned char *rawData);
    static size_t cryptedDataPointer(const unsigned char *rawData);
    static bool rawDataCrypted(const unsigned char *rawData);

private:
    void increaseSize(size_t elementSize);

    unsigned char	m_headerId;
    size_t			m_size;
    unsigned char	m_protocolId;
    unsigned char	m_data[c_ioDataMaxSize];
    size_t			m_headerSize;
    size_t			m_maxSize;
};

class eMUCORE_DECLSPEC scheduler_t {
public:
    scheduler_t();

    void insert(schedule_e::type_t type, const boost::function0<void> &callback, time_t delay);
    void worker();

    void synchronizer(synchronizer_t *synchronizer) {
        m_synchronizer = synchronizer;
    }

private:
    scheduler_t(const scheduler_t&);
    scheduler_t& operator=(const scheduler_t&);

    struct schedule_t {
        boost::function0<void>	m_callback;
        schedule_e::type_t		m_type;
        time_t					m_delay;
        time_t					m_lastExecuteTime;
    };

    std::vector<schedule_t> m_list;
    synchronizer_t			*m_synchronizer;
};
};

#pragma warning(default: 4251)
//#pragma warning(default: 4290)

#endif // eMU_CORE_CORE_H