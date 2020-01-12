#include <stdio.h>
#include <string.h>
#include <omnetpp.h>

using namespace omnetpp;

class Node : public cSimpleModule {
private:
    //Estado actual en el que se encuentra la máquina de estados.
    int state = 0;
    //Estados de la máquina de estados que regula el comportamiento del nodo.
    short idle = 0;
    short sending = 1;
    short waiting = 2;

    //Cola de transmisión de paquetes de información.
    cQueue *txQueue;
    //Cola de recepción de ACK/NACK.
    cQueue *ackQueue;

    //Canal de transmisión de paquetes entre el nodo y el destino.
    cDatarateChannel *channel;

    //Paquete en buffer a la espera de ser enviado correctamente.
    cPacket *bufferedPck;

    //Mensaje que indica que ha vencido el contador que controla la retransmisión automática de los paquetes.
    cMessage *timer;
    //Valor del contador que controla la retransmisión automática de los paquetes, expresado en segundos.
    float counter = 0.011;

    //Cambia el estado de la máquina de estados que regula el comportamiento del nodo a idle.
    void changeToIdle();
    //Cambia el estado de la máquina de estados que regula el comportamiento del nodo a sending.
    void changeToSending();
    //Cambia el estado de la máquina de estados que regula el comportamiento del nodo a waiting.
    void changeToWaiting();

    //Regula el comportamiento del nodo cuando se recibe el primer paquete después de estar la cola vacía, en el caso de usar un protoclo Stop&Wait.
    void idleSW(cMessage *msg);
    //Regula el comportamiento del nodo cuando se quiere enviar un paquete en la cola, en el caso de usar un protoclo Stop&Wait.
    //El argumento msg sólo es necesario si se gestiona un mensaje que acaba de llegar al nodo (que no esté en la cola). Si no, se sustituye por NULL.
    //Los argumentos ack y nack son flags que indican si se solicita avanzar en la cola o una retransmisión respectivamente.
    //Además, se define false, false como gestión de un nuevo paquete de información y true, true como gestión de mensaje ACK/NACK
    void sendingSW(cMessage *msg, bool ack, bool nack);
    //Regula el comportamiento del nodo cuando se espera a la recepción de la confirmación de envío, en el caso de usar un protoclo Stop&Wait.
    //El argumento info indica la naturaleza de msg: si es true, es un paquete de información; si es false, es ACK/NACK.
    void waitingSW(cMessage *msg, bool info);

    //Encola un mensaje en su correspondiente cola (información si info es true y ACK/NACK en caso contrario).
    void putPacketOnQueue(cMessage *msg, bool info);
    //Envía un paquete desde el nodo hacia el destino.
    void sendPckSW(cPacket *pck);

protected:
    //Inicializa los distintos componenetes del nodo.
    virtual void initialize() override;
    //Gobierna el comportamiento del nodo una vez se recibe un mensaje de cualquier tipo.
    virtual void handleMessage(cMessage *msg) override;
};

Define_Module(Node);

void Node::initialize() {
    //Inicializa las diferentes colas del nodo como colas tipo FIFO.
    txQueue = new cQueue("txQueue");
    ackQueue = new cQueue("ackQueue");

    //Inicializa el canal de transmisión de paquetes entre el nodo y el destino.
    channel = check_and_cast<cDatarateChannel *>(gate("out")->getTransmissionChannel());

    EV << "Node ready.\n";
}

void Node::handleMessage(cMessage *msg) {
    EV << "New packet arrived to node.\n";

    if(strcmp(msg->getName(), "ack")) { //Si NO es un mensaje de ACK

        if(!strcmp(msg->getName(), "timer")) { //Retransmisión por fin de contador
            EV << "Packet lost. Retransmission needed.\n";
            changeToSending();

            //Solicitud de retransmisión del paquete almacenado en memoria.
            sendingSW(NULL, false, true);
        }

        else if(strcmp(msg->getName(), "nack")) { //Si NO es un mensaje de NACK
            switch(state) {
            case 0:
                idleSW(msg);
                break;
            case 1:
                sendingSW(msg, false, false);
                break;
            case 2:
                waitingSW(msg, true);
                break;
            }
        }
        else { //Si ES un mensaje NACK
            switch(state) {
            case 0: //No tendría sentido que llegara algún ACK/NACK estando en idle, por tanto, no se gestiona.
                break;
            case 1:
                sendingSW(msg, true, true);
                break;
            case 2:
                waitingSW(msg, false);
                break;
            }
        }
    }

    else { //Si ES un mensaje ACK
        switch(state) {
        case 0: //No tendría sentido que llegara algún ACK/NACK estando en idle, por tanto, no se gestiona.
            break;
        case 1:
            sendingSW(msg, true, true);
            break;
        case 2:
            waitingSW(msg, false);
            break;
        }
    }
}


void Node::changeToIdle() {
    state = idle;
}

void Node::changeToSending() {
    state = sending;
}

void Node::changeToWaiting() {
    state = waiting;
}


void Node::idleSW(cMessage *msg) {
    //Encola el paquete que acaba de recibir
    putPacketOnQueue(msg, true);

    //Se pasa al siguiente estado
    changeToSending();

    //Se pasa a gestionar el estado de sending
    sendingSW(NULL, false, false);
}

void Node::sendingSW(cMessage *msg, bool ack, bool nack) {
    if(msg == NULL) { //Si se quiere iniciar los envíos de la cola
        if(ack == false && nack == false) { //Se trata de un paquete nuevo a enviar
            //Envía el siguiente paquete en cola
            bufferedPck = (cPacket *)txQueue->pop(); //Paquete al principio de la cola
            sendPckSW(bufferedPck); //Hay que enviar una copia del mensaje para poder simular correctamente el reenvío en caso de error

            changeToWaiting();
        }
        else if(ack == false, nack == true) { //Se está solicitando una retransmisión
            int rtx = bufferedPck->par("rtx").longValue();
            bufferedPck->par("rtx").setLongValue(rtx+=1);
            sendPckSW(bufferedPck); //Hay que enviar una copia del mensaje para poder simular correctamente el reenvío en caso de error

            changeToWaiting();
        }
        else { //Se trata de un ACK, por lo que hay que gestionar el siguiente envío en la cola
            delete(bufferedPck); //Se borra el paquete en buffer

            if(txQueue->empty()) { //Si la cola de transmisión está vacía, salimos de aquí
               changeToIdle();
            }
            else { //Si no, enviamos el siguiente
                bufferedPck = (cPacket *)txQueue->pop(); //Paquete al principio de la cola
                sendPckSW(bufferedPck); //Hay que enviar una copia del mensaje para poder simular correctamente el reenvío en caso de error

                changeToWaiting();
            }
        }
    }
    else { //Si se llega con un paquete nuevo
        if(ack == false && nack == false)
            putPacketOnQueue(msg, true);
        else if(ack == true && nack == true)
            putPacketOnQueue(msg, false);
    }
}

void Node::waitingSW(cMessage *msg, bool info) {
    if(info == true) {
        putPacketOnQueue(msg, true);
    }
    else {
        //Se cancela el evento de contador, ya que el que haya respuesta significa que no se ha perdido el paquete.
        cancelEvent(timer);
        putPacketOnQueue(msg, false);

        //Se recupera el mensaje de respuesta.
        cMessage *confirm = (cMessage *)ackQueue->pop();
        changeToSending();

        //Se escoge el siguiente paso a dar.
        if(!strcmp(confirm->getName(), "ack")) //Si es un ACK.
            sendingSW(NULL, true, false);
        if(!strcmp(confirm->getName(), "nack")) //Si es un ACK.
            sendingSW(NULL, false, true);
    }
}


void Node::putPacketOnQueue(cMessage *msg, bool info) {
    if(info == true) { //Paquete de información
        txQueue->insert(check_and_cast<cPacket *>(msg));
    }
    else { //Respuesta al envío de un paquete de información.
        cPacket *res = check_and_cast<cPacket *>(msg);
        EV << "Response received for pck " << res->par("id").longValue() << ".\n";
        ackQueue->insert(res);
    }
}

void Node::sendPckSW(cPacket *pck) {
    cPacket *packetToSend = new cPacket(*pck); //Genera una copia del paquete a enviar, que será lo que realmente se mande, de forma que no se pierda el paquete original
    EV << "Sending pck " << packetToSend->par("id").longValue() << ".\n";
    send(packetToSend, "out");

    //Inicia el contador que controlará la possible pérdida del paquete recién enviado.
    timer = new cMessage("timer");
    scheduleAt(simTime() + counter, timer);
}
