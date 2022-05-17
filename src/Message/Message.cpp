#include "Message.h"

Message::Message(String topic, String msg, String payload, uint32_t from){
    this->topic = TopicAdapter::convertTopicString(topic);
    this->msg = msg;
    this->payload = payload;
    this->from = from;
}

String Message::toString(){
    String msg = "";
    msg += "Topic: " + this->topic;
    msg += "Msg: " + this->msg;
    msg += "Payload: " + this->payload;
    msg += "From: " + this->from;
    return msg;
}

String Message::getMsg(){
    return this->msg;
}

uint32_t Message::getFrom(){
    return this->from;
}

String Message::getPayload(){
    return this->payload;
}

TopicType Message::getTopic(){
    return this->topic;
}