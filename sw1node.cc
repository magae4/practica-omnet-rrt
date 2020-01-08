/*#include <stdio.h>
#include <string.h>
#include <omnetpp.h>

using namespace omnetpp;

class Source : public cSimpleModule {
  protected:
    virtual void initialize() override;
};

Define_Module(Source);

void Source::initialize() {
    EV << "Source ready. Sending initial message\n";
    cPacket *pck= new cPacket("pck1", (short)0, (int64_t)48);
    send(pck, "out");
}

class Destiny : public cSimpleModule {
  protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
};

Define_Module(Destiny);

void Destiny::initialize() {
    EV << "Destiny ready.\n";
}

void Destiny::handleMessage(cMessage *msg) {
    EV << "Message arrived.\n";

    cPacket *ack = new cPacket("ack", (short)0, (int64_t)0);
    cMessage *asdf = new cPacket("asdf", (short)0, (int64_t)0);

    send(ack, "out");
}

class Node : public cSimpleModule {
  protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
};

Define_Module(Node);

void Node::initialize() {
    EV << "Node ready.\n";
}

void Node::handleMessage(cMessage *msg) {
    EV << "Message arrived.\n";

    if(strcmp(msg->getName(), "ack")) {
        send(msg, "out");
        //SEGÚN MANDE, DEBE INICIAR EL CONTADOR
    }
    else
        EV << "FIN";

    //FALTARÍA GESTIONAR EL NACK
}*/


