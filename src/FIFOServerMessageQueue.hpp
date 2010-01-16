#ifndef _CBR_FIFOSENDQUEUE_HPP
#define _CBR_FIFOSENDQUEUE_HPP

#include "FIFOQueue.hpp"
#include "ServerMessageQueue.hpp"
#include "FairQueue.hpp"
#include "NetworkQueueWrapper.hpp"

namespace CBR {
class FIFOServerMessageQueue:public ServerMessageQueue {
    typedef FIFOQueue<Message, ServerID> FIFOSendQueue;
    FIFOSendQueue mQueue;

    Time mLastServiceTime;

    uint32 mSendRate;
    uint32 mRemainderSendBytes;
    Time mLastSendEndTime; // the time at which the last send ended, if there are messages that are too big left in the queue

public:
    FIFOServerMessageQueue(SpaceContext* ctx, Network* net, ServerIDMap* sidmap, Listener* listener, uint32 send_bytes_per_second);

    virtual bool addMessage(Message* msg);
    virtual bool canAddMessage(const Message* msg);
    virtual void service();

    virtual void addInputQueue(ServerID sid, float weight);
    virtual void updateInputQueueWeight(ServerID sid, float weight);
    virtual void removeInputQueue(ServerID sid);
  private:
    virtual void networkReadyToSend(const Address4& from);
};
}
#endif
