#include <stdio.h>
#include <string.h>
#include <omnetpp.h>

using namespace omnetpp;

class Destiny : public cSimpleModule {
  protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;

  private:
    //Envía de vuelta un mensaje ACK
    void sendACK();
    //Envía de vuelta un mensaje NACK
    void sendNACK();
};

Define_Module(Destiny);

//Inicializa los distintos componenetes del nodo
void Destiny::initialize() {
    EV << "Destiny ready.\n";
}

//Gobierna el comportamiento del nodo una vez se recibe un mensaje de cualquier tipo
void Destiny::handleMessage(cMessage *msg) {
    EV << "Message arrived.\n";

    cPacket *pck = check_and_cast<cPacket *>(msg);
    if(pck->hasBitError()) //En caso de que hubiera error
        sendNACK();
    else //En caso de que no lo hubiera
        sendACK();
}

void Destiny::sendACK() {
    EV << "No errors found. Sending ACK...\n";

    cPacket *ack = new cPacket("ack", (short)0, (int64_t)0);
    send(ack, "out");
}

void Destiny::sendNACK() {
    EV << "Errors found. Sending NACK...\n";

    cPacket *nack = new cPacket("nack", (short)0, (int64_t)0);
    send(nack, "out");
}
