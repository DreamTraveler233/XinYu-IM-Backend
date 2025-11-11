#ifndef __CIM_APP_TALK_SESSION_HPP__
#define __CIM_APP_TALK_SESSION_HPP__

#include "dao/talk_session_dao.hpp"
#include "result.hpp"

namespace CIM::app {

class TalkService {
   public:
    static TalkSessionListResult getSessionListByUserId(const uint64_t user_id);

    static VoidResult setSessionTop(const uint64_t to_from_id, const uint8_t talk_mode,
                                    const uint8_t action);
};

}  // namespace CIM::app

#endif  // __CIM_APP_TALK_SESSION_HPP__