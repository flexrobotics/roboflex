/**
 * Super-basic test. Inherits Message to create a custom 
 * message type, inherits RunnableNode to create a custom
 * producer node (which signals our custom message), and 
 * inherits Node to create a custom consumer node (which
 * receives our custom message).
 */

#include <iostream>
#include "core/core.h"

using namespace roboflex::core;

class MyStringMessage: public Message {
public:
    MyStringMessage(Message& other): Message(other) {}

    MyStringMessage(const string& message):
        Message("basic_0", "MyStringMessage")
    {
        flexbuffers::Builder fbb = get_builder();
        WriteMapRoot(fbb, [&](){
            fbb.String("msg", message);
        });
    }

    const string message() const {
        return get_root_as_map()["msg"].AsString().str();
    }

    void print_on(ostream& os) const override {
        os << "<MyStringMessage msg: \"" << message() << "\" ";
        Message::print_on(os);
        os << ">";
    }
};

class ProducerNode: public RunnableNode {
public:
    ProducerNode(): RunnableNode("Producer") {}
    void child_thread_fn() override {
        for (int i=0; i<4; i++) {
            sleep_ms(500);
            this->signal(std::make_shared<MyStringMessage>("Hello, world!"));
        }
    }
};

class ConsumerNode: public Node {
public:
    ConsumerNode(): Node("Consumer") {}
    void receive(MessagePtr m) override {
        std::cout << "ConsumerNode received: " << *m << std::endl;
    }
};

int main() {
    ProducerNode p;
    ConsumerNode c;
    std::cout << "Producer: " << p.to_string() << std::endl;
    std::cout << "Consumer: " << c.to_string() << std::endl;
    p > c; // same as p.connect(c);
    p.start();
    p.stop();
    std::cout << "DONE" << std::endl;
    return 0;
}