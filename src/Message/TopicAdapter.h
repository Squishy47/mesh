#include "Arduino.h"
#include "TopicType.h"

class TopicAdapter {
    public:
        static TopicType convertTopicString(String topic);
};