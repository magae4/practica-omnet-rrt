#include <stdio.h>
#include <string.h>
#include <omnetpp.h>

using namespace omnetpp;

class Source : public cSimpleModule {
  protected:
    //Inicializa los distintos componenetes del nodo
    virtual void initialize() override;
};

Define_Module(Source);

void Source::initialize() {
    EV << "Source ready.\n";

    //Paquete 1
    cPacket *pck= new cPacket("pck1", (short)0, (int64_t)1000);
    send(pck, "out");
    //Paquete 2
    cPacket *pck2= new cPacket("pck2", (short)0, (int64_t)1000);
    send(pck2, "out");
    //Paquete 3
    cPacket *pck3= new cPacket("pck3", (short)0, (int64_t)1000);
    send(pck3, "out");
    //Paquete 4
    cPacket *pck4= new cPacket("pck4", (short)0, (int64_t)1000);
    send(pck4, "out");
}
