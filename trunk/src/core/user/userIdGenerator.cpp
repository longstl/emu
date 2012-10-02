#include <user/userIdGenerator.hpp>

namespace eMU {
namespace core {
namespace user {

idGenerator_t::idGenerator_t(size_t idsPoolSize) {
    for(size_t i = 0; i < idsPoolSize; ++i) {
        this->insert(static_cast<int16>(i));
    }
}

int16 idGenerator_t::get() {
    if(!idsPool_.empty()) {
        int16 id = idsPool_.front();
        idsPool_.pop();
        return id;
    }
    
    return invalidId_c;
}

void idGenerator_t::insert(int16 id) {
    idsPool_.push(id);
}

}
}
}