#include <Arduino.h>
#include "TopicType.h"
#include "TopicAdapter.h"

class Message{
    TopicType topic;
    String msg;
    String payload;
    uint32_t from;

  public:
    Message(String topic, String msg, String payload, uint32_t from);
    ~Message();


    String toString();
    TopicType getTopic();
    String getMsg();
    String getPayload();
    uint32_t getFrom();
};

