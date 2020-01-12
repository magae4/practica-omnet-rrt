//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

//ELEMENTOS

//Origen de los paquetes
simple Source
{
    parameters:
        @display("i=block/tx");
    gates:
        output out;
}

//Destino de los paquetes
simple Destiny
{
    parameters:
        @display("i=block/rx");
    gates:
        input in;
        output out;
}

//Nodo intermedio
simple Node
{
    @display("i=block/routing");
    gates:
        input in;
        output out;

        input out_response; //Es el lugar por el que llegan los ACK o NACK del destino.
}

//RED A SIMULAR

//Origen y destino conectados a través de un único nodo intermedio que se comunica con el destino mediante un protocolo S&W
network SW1Node
{
    types:
        channel Channel extends ned.DatarateChannel
        {
            datarate = 100kbps;
	        ber = 0.0005;
        }

    submodules:
        S: Source {
            @display("p=51,144");
        }
        D: Destiny {
            @display("p=217,144");
        }
        N1: Node {
            @display("p=132,144");
        }
    connections:
        S.out --> N1.in;

        N1.out --> Channel --> D.in;
        N1.out_response <-- D.out;
}

