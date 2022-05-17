#include "TopicAdapter.h"
#include "TopicType.h"

TopicType TopicAdapter::convertTopicString(String topic){
    if(topic == "BROADCAST"){
        return BROADCAST;
    }
    else if(topic == "SINGLE"){
        return SINGLE;
    }
    else if(topic == "GATEWAY"){
        return GATEWAY;
    }
    else {
        return BROADCAST;
    }
}