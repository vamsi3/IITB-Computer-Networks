#include "bridge.h"
#include <algorithm>
#include <climits>

//// spanning tree protocol functionality

// BPDU struct definitions

Bridge::Port::Port(char port_id): port_id(port_id), best_received_bpdu(BPDU()) {}
Bridge::Port::Port(char port_id, BPDU best_received_bpdu): port_id(port_id), best_received_bpdu(best_received_bpdu) {}
bool Bridge::Port::operator<(const Port &other) const {
	if (port_id != other.port_id) return port_id < other.port_id;
	return best_received_bpdu < other.best_received_bpdu;
}

// BPDU struct definitions

Bridge::BPDU::BPDU():
	root_id(INT_MAX), root_path_cost(INT_MAX),  bridge_id(INT_MAX), port_id(CHAR_MAX) {}
Bridge::BPDU::BPDU(int root_id, int root_path_cost, int bridge_id, char port_id):
	root_id(root_id), root_path_cost(root_path_cost),  bridge_id(bridge_id), port_id(port_id) {}
bool Bridge::BPDU::operator<(const BPDU &other) const {
	if (root_id != other.root_id) return root_id < other.root_id;
	else if (root_path_cost != other.root_path_cost) return root_path_cost < other.root_path_cost;
	else if (bridge_id != other.bridge_id) return bridge_id < other.bridge_id;
	else if (port_id != other.port_id) return port_id < other.port_id;
	else return false;
}

// Bridge definitions

Bridge::Bridge(int id, bool trace): trace(trace), root_id(id), root_path_cost(0), id(id), root_port_id(0) {
	best_received_bpdu = BPDU(id, -1, INT_MAX, 0);
}

void Bridge::AddAdjacentPort(char port_id) {
	this->adjacent_ports.insert(Port(port_id));
}

void Bridge::UpdateStatus(Bridge::BPDU bpdu) {
	best_received_bpdu = bpdu;
	root_id = best_received_bpdu.root_id;
	root_path_cost = best_received_bpdu.root_path_cost + 1;
	root_port_id = best_received_bpdu.port_id;
}

Bridge::BPDU Bridge::GetBPDU(char port_id) const {
	return BPDU(root_id, root_path_cost, id, port_id);
}

std::set<Bridge::Port>::iterator Bridge::GetPort(char port_id) const {
	return find_if(this->adjacent_ports.begin(), this->adjacent_ports.end(), [&port_id](Bridge::Port port) {return port.port_id == port_id;});
}

std::set<char> Bridge::GetDesignatedPorts() const {
	std::set<char> designated_ports;
	for (auto &port: this->adjacent_ports) {
		if (this->GetBPDU(port.port_id) < port.best_received_bpdu) {
			designated_ports.insert(port.port_id);
		}
	}
	return designated_ports;
}

void Bridge::UpdateOnReceivedBPDU(int time) {
	for (auto bpdu_message_iterator = received_bpdu.begin(); bpdu_message_iterator != received_bpdu.end(); bpdu_message_iterator++) {
		auto bpdu_message = *bpdu_message_iterator;
		if (bpdu_message.first == time) {
			auto bpdu = bpdu_message.second;
			auto port_iterator = GetPort(bpdu.port_id);
			if (bpdu < port_iterator->best_received_bpdu) {
				adjacent_ports.erase(port_iterator);
				adjacent_ports.insert(Port(bpdu.port_id, bpdu));
			}
			if (bpdu < this->best_received_bpdu) this->UpdateStatus(bpdu);
			received_bpdu.erase(bpdu_message_iterator);
			// printing received BPDU trace
			if (trace) std::cout << time << " B" << id << " r (B" << bpdu.root_id << ", " << bpdu.root_path_cost << ", B" << bpdu.bridge_id << ")\n";
		}
	}
}

std::set< std::pair<int, Bridge::BPDU> > Bridge::GenerateBPDU(int time) const {
	std::set< std::pair<int, Bridge::BPDU> > generated_bpdu;
	bool atleast_one_insertion = false;
	for (const char &designated_port_id: this->GetDesignatedPorts()) {
		generated_bpdu.insert(std::make_pair(time + 1, this->GetBPDU(designated_port_id)));
		atleast_one_insertion = true;
	}
	// printing sent BPDU trace
	if (trace and atleast_one_insertion) std::cout << time << " B" << id << " s (B" << root_id << ", " << root_path_cost << ", B" << id << ")\n";
	return generated_bpdu;
}

void Bridge::AddToReceivedBPDU(std::pair<int, BPDU> bpdu_message) {
	received_bpdu.insert(bpdu_message);
}

void Bridge::PrintPorts() const {
	auto designated_ports = this->GetDesignatedPorts();
	std::cout << 'B' << id << ": ";
	for (const auto &port: adjacent_ports) {
		std::string port_status;
		if (designated_ports.find(port.port_id) != designated_ports.end()) port_status = "DP";
		else if (port.port_id == root_port_id) port_status = "RP";
		else port_status = "NP";
		std::cout << port.port_id << '-' << port_status << ' ';
	}
	std::cout << std::endl;
}


//// learning bridges functionality

// Data struct definitions

Bridge::Data::Data(): time(-1), host1_id(0), host2_id(0), port_id('0') {}
Bridge::Data::Data(int time, int host1_id, int host2_id, char port_id):
	time(time), host1_id(host1_id), host2_id(host2_id), port_id(port_id) {}
bool Bridge::Data::operator<(const auto &other) const {
	if (time != other.time) return time < other.time;
	else if (port_id != other.port_id) return port_id < other.port_id;
	else if (host1_id != other.host1_id) return host1_id < other.host1_id;
	else if (host2_id != other.host2_id) return host2_id < other.host2_id;
	else return false;
}

// Bridge definitions

void Bridge::AddForwardingRow(int host_id, char port_id) {
	forwarding_table[host_id] = port_id;
}

std::set<char> Bridge::GetForwardingPorts() const {
	auto forwarding_ports = GetDesignatedPorts();
	if (root_id != id) forwarding_ports.insert(root_port_id);
	return forwarding_ports;
}

void Bridge::SetNumOfHosts(int num_of_hosts) {
	forwarding_table.resize(num_of_hosts + 1, 0);
}

void Bridge::UpdateLanToBridges(std::map<char, std::set<int> > &lan_to_bridges) const {
	auto forwarding_ports = GetForwardingPorts();
	for (const auto &port: forwarding_ports) {
		lan_to_bridges[port].insert(id);
	}
}

void Bridge::AddData(Bridge::Data data) {
	this->data = data;
}

void Bridge::UpdateOnData(int time) {
	if (time == this->data.time) {
		if (forwarding_table[data.host1_id] == 0) {
			forwarding_table[data.host1_id] = data.port_id;
		}
		// printing received Data transfer trace
		if (trace) std::cout << time << " B" << id << " r " << data.port_id << " --> " << '-' << std::endl;
	}
}

bool Bridge::IsDataPresent(int time) const {
	return (time == this->data.time);
}

std::set<Bridge::Data> Bridge::GenerateData(int time) const {
	std::set<Bridge::Data> generated_data; std::set<char> forwarding_ports;
	char forwarding_port = forwarding_table[this->data.host2_id];
	if (forwarding_port != 0) forwarding_ports.insert(forwarding_port);
	else forwarding_ports = GetForwardingPorts();
	for (const auto &port_id: forwarding_ports) {
		if (port_id != data.port_id) {
			generated_data.insert(Data(time + 1, data.host1_id, data.host2_id, port_id));
			// printing sent Data transfer trace
			if (trace) std::cout << time << " B" << id << " s " << data.port_id << " --> " << port_id << std::endl;
		}
	}
	return generated_data;
}

void Bridge::PrintForwardingTable() const {
	std::cout << "B" << this->id << ": " << std::endl; 
	std::cout << "HOST ID | FORWARDING PORT" << std::endl;
	for (int host_id = 0; host_id < forwarding_table.size(); host_id++) {
		if (forwarding_table[host_id] != 0) std::cout << "H" << host_id << " | " << forwarding_table[host_id] << " " << std::endl;
	}
}
