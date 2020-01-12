#include <stdio.h>
#include <string.h>
#include <omnetpp.h>
#include <string.h>

using namespace omnetpp;

class Source : public cSimpleModule {
private:
    //Identificador del paquete.
    int pckId = 0;

    //Mensaje autoenviado que indica que se deben enviar un nuevo paquete al nodo.
    cMessage *timerMessage;

    //Crea y rellena diferentes parámetros del paquete que se le pase.
    void fillPackage(cPacket *pck);

protected:
    //Inicializa los distintos componenetes del nodo.
    virtual void initialize() override;
    //Gobierna el comportamiento del nodo una vez se recibe un mensaje de cualquier tipo.
    virtual void handleMessage(cMessage *msg);
};

Define_Module(Source);

void Source::initialize() {
    EV << "Source ready.\n";

    //Envío del primer paquete.
    timerMessage = new cMessage("timer");
    scheduleAt(simTime(), timerMessage);
}

void Source::handleMessage(cMessage *msg) {
    //Se crea y prepara el siguiente paquete a enviar, con una longotud arbritaria de 1000 bits.
    cPacket *pck= new cPacket("pck", (short)0, (int64_t)1000);
    fillPackage(pck);

    //Envío del paquete.
    send(pck, "out");

    //Se programa en envío del siguiente paquete, siguiendo una distribución exponencial de media 0.01 segundos.
    scheduleAt(simTime() + exponential(0.01), timerMessage);
}

void Source::fillPackage(cPacket *pck) {
    //Identificador del paquete.
    pck->addPar("id");
    //Nº de retransmisiones.
    pck->addPar("rtx");
    //Tiempo de llegada.
    pck->addPar("arrival");

    //Se asigna valor a los parámetros anteriores.
    pck->par("id").setLongValue(pckId++);
    pck->par("rtx").setLongValue(0);
    pck->par("arrival").setLongValue(simTime().dbl()); //Se obtiene el momento de simulación actual y se establece como valor de llegada.
}
