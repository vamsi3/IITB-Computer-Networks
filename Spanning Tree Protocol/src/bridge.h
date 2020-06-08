#include <iostream>
#include <vector>
#include <set>
#include <map>

class Bridge {
public:
	// spanning tree protocol functionality - BPDU is Bridge Protocol Data Unit
	struct BPDU {
		int root_id, root_path_cost, bridge_id; char port_id;
		BPDU();
		BPDU(int root_id, int root_path_cost, int bridge_id, char port_id);
		bool operator<(const BPDU &other) const;
	};
	// for describing a port
	struct Port {
		char port_id; BPDU best_received_bpdu;
		Port(char port_id);
		Port(char port_id, BPDU best_received_bpdu);
		bool operator<(const Port &other) const;
	};
	// learning bridges functionality
	struct Data {
		int time, host1_id, host2_id; char port_id;
		Data();
		Data(int time, int host1_id, int host2_id, char port_id);
		bool operator<(const auto &other) const;
	};
private:
	bool trace;

	// spanning tree protocol functionality
	int root_id, root_path_cost, id; char root_port_id;
	BPDU best_received_bpdu;
	std::set<Port> adjacent_ports;
	std::set< std::pair<int, BPDU> > received_bpdu;
	void UpdateStatus(BPDU bpdu);
	BPDU GetBPDU(char port_id) const;
	std::set<Port>::iterator GetPort(char port_id) const;
	std::set<char> GetDesignatedPorts() const;

	// learning bridges functionality
	std::vector<char> forwarding_table;
	Data data;
	void AddForwardingRow(int host_id, char port_id);
	std::set<char> GetForwardingPorts() const;

public:
	Bridge(int id, bool trace);
	void AddAdjacentPort(char port_id);

	// spanning tree protocol functionality
	void UpdateOnReceivedBPDU(int time);
	std::set< std::pair<int, BPDU> > GenerateBPDU(int time) const;
	void AddToReceivedBPDU(std::pair<int, BPDU> bpdu_message);
	void PrintPorts() const;

	// learning bridges functionality
	void SetNumOfHosts(int num_of_hosts);
	void UpdateLanToBridges(std::map<char, std::set<int> > &lan_to_bridges) const;
	void AddData(Data data);
	void UpdateOnData(int time);
	bool IsDataPresent(int time) const;
	std::set<Data> GenerateData(int time) const;
	void PrintForwardingTable() const;
};