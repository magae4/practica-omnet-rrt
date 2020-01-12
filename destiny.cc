#include <stdio.h>
#include <string.h>
#include <omnetpp.h>

using namespace omnetpp;

class Destiny : public cSimpleModule {
private:
    //"Probabilidad" de pérdida de paquetes de información del canal, expresado en valores de 1 a 100.
    int losingProbability = 50;

    //Histogramas de los parámetros que se van a dibujar: tiempo entre llegadas, retransmisiones y retardo del paquete de origen a destino, respectivamente.
    cHistogram interarrivalsHistogram;
    cHistogram rtxHistogram;
    cHistogram delayHistogram;
    //Vectores para los histogramas anteriores.
    cOutVector interarrivalsVector;
    cOutVector rtxVector;
    cOutVector delayVector;

    //Momento de la llegada del anterior paquete. Usado para medir el tiempo entre llegadas.
    simtime_t lastArrival;

    //Envía de vuelta un mensaje ACK
    void sendACK(cPacket *pck);
    //Envía de vuelta un mensaje NACK
    void sendNACK(cPacket *pck);

    //Genera un número aleatorio entre 1 y 100.
    double generateRandomNumber();
    //Simula la pérdida de paquetes en el canal durante la transmisión desde el nodo al destino.
    bool simulateLost();

protected:
    //Inicializa los distintos componenetes del nodo.
    virtual void initialize() override;
    //Gobierna el comportamiento del nodo una vez se recibe un mensaje de cualquier tipo.
    virtual void handleMessage(cMessage *msg) override;
    //Se encarga de las funciones a realizar cuando se termine la simulación.
    virtual void finish() override;
};

Define_Module(Destiny);

void Destiny::initialize() {
    EV << "Destiny ready.\n";

    //Inicializa la última llegada a 0.
    lastArrival = simTime();

    //Configura los diferentes histogramas y sus vectores.
    interarrivalsHistogram.setName("Interarrival times");
    rtxHistogram.setName("Retransmissions number");
    delayHistogram.setName("Packet delay");

    interarrivalsVector.setName("arrivals");
    interarrivalsVector.setInterpolationMode(cOutVector::NONE);

    rtxVector.setName("retransmissions");
    rtxVector.setInterpolationMode(cOutVector::NONE);

    delayVector.setName("delay");
    delayVector.setInterpolationMode(cOutVector::NONE);
}

void Destiny::handleMessage(cMessage *msg) {
    //Actualiza el histograma de tiempo entre llegadas.
    simtime_t d = simTime() - lastArrival;
    interarrivalsHistogram.collect(d);
    interarrivalsVector.record(1);

    //Al llegar un nuevo paquete, actualiza el tiempo de llegada del último.
    lastArrival = simTime();

    if(!simulateLost()) { //En caso de que no se haya perdido el paquete
        EV << "Message arrived.\n";
        cPacket *pck = check_and_cast<cPacket *>(msg);
        if(pck->hasBitError()) //En caso de que hubiera error
            sendNACK(pck);
        else { //En caso de que no lo hubiera
            //Los otros dos histogramas deben actualizarese únicamente cuando ha llegado un paquete.
            rtxHistogram.collect(pck->par("id").longValue());
            rtxVector.record(1);

            delayHistogram.collect(simTime().dbl() - pck->par("arrival").longValue());
            delayVector.record(1);

            sendACK(pck);
        }
    }
}

void Destiny::sendACK(cPacket *pck) {
    EV << "No errors found on packet " << pck->par("id").longValue() << ". Sending ACK...\n";

    //Genera el paquete con el ACK, indicando a qué paquete de información hace referencia.
    cPacket *ack = new cPacket("ack", (short)0, (int64_t)0);
    ack->addPar("id");
    ack->par("id").setLongValue(pck->par("id").longValue());

    send(ack, "out");
}

void Destiny::sendNACK(cPacket *pck) {
    EV << "Errors found on packet " << pck->par("id").longValue() << ". Sending NACK...\n";

    //Genera el paquete con el NACK, indicando a qué paquete de información hace referencia.
    cPacket *nack = new cPacket("nack", (short)0, (int64_t)0);
    nack->addPar("id");
    nack->par("id").setLongValue(pck->par("id").longValue());

    send(nack, "out");
}

void Destiny::finish() {
    //Saca las estadísticas finales.
    recordStatistic(&interarrivalsHistogram);
    recordStatistic(&rtxHistogram);
    recordStatistic(&delayHistogram);
}

bool Destiny::simulateLost() {
    if(generateRandomNumber() < losingProbability) //Existe error.
        return true;
    else //No hay error.
        return false;
}

double Destiny::generateRandomNumber() {
    srand(time(NULL)); //La semilla para el generador se saca del tiempo actual.
    return rand() % 100;
}
