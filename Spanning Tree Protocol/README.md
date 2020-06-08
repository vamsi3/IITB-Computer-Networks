# Spanning Tree Protocol Implementation

> This project implements a simplified model of the Spanning Tree Protocol (as described in IEEE.802.1D) in C++14.

It was made as a solution to the [Problem Statement](problem_statement.pdf) given as a part of the CS252 (Computer Networks Lab) course requirement at IIT Bombay.

## Code Description

To run the code, simply compile and execute [spanningTree.cpp](src/spanningTree.cpp) with other class definition files in the same directory.

This project consists of three files _i.e._ [spanningTree.cpp](src/spanningTree.cpp) which implements the main functionality and [bridge.h](src/bridge.h), [bridge.cpp](src/bridge.cpp) which implement a `Bridge` class, an object of which represents a bridge in the network given.

### spanningTree.cpp

This file contains:

* `ReadNetworkGraph()`, `ReadHostToLan()`, `ReadDataTransfers()` which read the input as three parts.
* `SpanningTreeProtocol()` which implements the Spanning Tree Protocol on the given input (stored as `network_graph`).
* `LearningBridges()` which send data while learning the LAN to which various hosts are connected.
* `lan_to_bridges` which stores all the bridges present (connected) on a LAN.
* `host_to_lan` stores the LAN to which a host is connected.
* `data_transfers` stores the `pair`s of Host IDs which are sending and receiving a data packet respectively.

### bridge.h and bridge.cpp

These files contain functions and variables whose names are mostly self explanatory. However, I shall describe a few important `public struct`s and the idea of the protocol as a whole.

* Struct `BPDU` _(BPDU stands for Bridge Protocol Data Unit)_ denotes the 'Configuration Messages' it sends as a part of the protocol to determine the 'spanning tree'.
* Struct `Port` contains the ID of the port and the best BPDU it the bridge receives _via._ that port yet. (Here *best* is used as defined clearly in the protocol itself)
* Struct `Data` has it's importance during the Learning Bridges Protocol where this represents an 'empty' (dataless, Aw! the irony :P) packet that is sent from one host to another _via_. various LANs.

## Algorithm for Spanning Tree Protocol

Each bridge listens on all its ports to BPDUs sent by other bridges. If a bridge receives a BPDU that advertises a 'better' path than advertised in its own BPDU, the bridge updates its BPDU. To determine if a received BPDU advertises a better path, the bridge compares the received BPDU to its own BPDU. If the root ID in the received BPDU is smaller than the root ID of the bridge, the received BPDU is seen as advertising a better path. If the root IDs are identical, the BPDU with the lower root path cost advertises a better path. If both the root ID and root path cost are identical, then the BPDU with the lower bridge ID is seen as advertising a better path. Finally, if root ID, root path cost, and bridge ID are all identical, then the BPDU with the lowest port ID is interpreted as advertising a better BPDU.

When a bridge with BPDU (R1, C1, B1, P1) receives a BPDU (R2, C2, B2, P2) and the received BPDU advertises a better path, the bridge updates its own BPDU to (R2, C2+increment, B1, P1). The increment value is a configuration parameter that accounts for the cost increase of the path due to bridge B1. When the increment value is set to 1 on all bridges, then the bridges establish a minimum hop route to the root bridge. The increment can also be set to account for the data rate of a LAN. For example, to make a path on a 100 Mbps LAN more desirable than on a 10 Mbps LAN, the 10 Mbps LAN can be assigned a larger increment value.

A bridge transmits its BPDU on a port, only if its BPDU advertises a better route than any of the BPDUs received on that port. In this case, the bridge assumes that it is the designated bridge for the LAN to which the port connects, and the port that connects the bridge to the LAN is called the designated port of the LAN. A bridge that is not the designated bridge for a LAN does not send BPDUs on that LAN.

On each bridge, the port where the bridge receives the BPDU that advertises the best route is labeled the root port.

_(Algorithm sourced from [this document](http://www.comm.utoronto.ca/~jorg/teaching/ece461/handouts/spt.pdf) by Prof. Jörg Liebeherr at University of Toronto)_

## Author

* **Vamsi Krishna Reddy Satti** - [vamsi3](https://github.com/vamsi3)

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details

