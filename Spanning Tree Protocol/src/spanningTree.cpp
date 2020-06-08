#include "bridge.cpp"
#include <sstream>
using namespace std;

// all the data input, read and parsing functions

void ReadNetworkGraph(const int &num_of_bridges, const bool &trace, vector<Bridge> &network_graph, map< char, vector<int> > &lan_to_bridges) {
	network_graph.reserve(num_of_bridges+1); network_graph.push_back(Bridge(0, trace));
	for (int bridge_num = 1; bridge_num <= num_of_bridges; bridge_num++) {
		network_graph.push_back(Bridge(bridge_num, trace));
	}
	cin.ignore();
	for (int read_bridge_count = 0; read_bridge_count < num_of_bridges; read_bridge_count++) {
		string line, input_word; getline(cin, line); istringstream iss(line);
		bool first = true; int bridge_num;
		while (iss >> input_word) {
			if (first) {
				bridge_num = stoi(input_word.substr(1, input_word.length()-2));
				first = false;
			} else {
				network_graph[bridge_num].AddAdjacentPort(input_word[0]);
				lan_to_bridges[input_word[0]].push_back(bridge_num);
			}
		}
	}
	for (auto &lan: lan_to_bridges) {
		sort(lan.second.begin(), lan.second.end());
	}
}

void ReadHostToLan(const int &num_of_lans, vector<char> &host_to_lan) {
	for (int read_lan_count = 0; read_lan_count < num_of_lans; read_lan_count++) {
		string line, input_word; getline(cin, line); istringstream iss(line);
		bool first = true; char lan;
		while (iss >> input_word) {
			if (first) {
				lan = input_word[0];
				first = false;
			} else {
				int host_id = stoi(input_word.substr(1, input_word.length()-1));
				if (host_id >= host_to_lan.size()) host_to_lan.resize(host_id+1);
				host_to_lan[host_id] = lan;
			}
		}
	}
}

void ReadDataTransfers(const int &num_of_data_transfers, vector< pair<int, int> > &data_transfers) {
	for (int i = 0; i < num_of_data_transfers; i++) {
		string host1, host2; cin >> host1 >> host2;
		int host1_value = stoi(host1.substr(1, host1.length()-1));
		int host2_value = stoi(host2.substr(1, host2.length()-1));
		data_transfers.push_back(make_pair(host1_value, host2_value));
	}
}

// SpanningTreeProtocol implementation

void SpanningTreeProtocol(vector<Bridge> &network_graph, map< char, vector<int> > &lan_to_bridges) {
	const int num_of_bridges = network_graph.size() - 1;
	int time = 0;
	while (time < 100) {
		for (int bridge_num = 1; bridge_num <= num_of_bridges; bridge_num++) {
			network_graph[bridge_num].UpdateOnReceivedBPDU(time);
		}
		for (int bridge_num = 1; bridge_num <= num_of_bridges; bridge_num++) {
			auto generated_messages =  network_graph[bridge_num].GenerateBPDU(time);
			for (auto const &message: generated_messages) {
				auto bpdu = message.second;
				for (const int &receiver_bridge_num: lan_to_bridges[bpdu.port_id]) {
					if (receiver_bridge_num != bridge_num) {
						network_graph[receiver_bridge_num].AddToReceivedBPDU(message);
					}
				}
			}
		}
		time++;
	}
	for (int bridge_num = 1; bridge_num <= num_of_bridges; bridge_num++) {
		auto bridge = network_graph[bridge_num];
		bridge.PrintPorts();
	}
}

// LearningBridges implementation

void LearningBridges(vector<Bridge> &network_graph, const vector<char> &host_to_lan, const vector< pair<int, int> > &data_transfers) {
	const int num_of_bridges = network_graph.size() - 1;
	int num_of_hosts = host_to_lan.size()-1;
	int global_time = 0;
	map<char, set<int> > lan_to_bridges;
	for (int bridge_num = 1; bridge_num <= num_of_bridges; bridge_num++) {
		network_graph[bridge_num].SetNumOfHosts(num_of_hosts);
		network_graph[bridge_num].UpdateLanToBridges(lan_to_bridges);
	}
	for (const auto &data_transfer: data_transfers) {
		global_time += 100;
		int host1_id = data_transfer.first, host2_id = data_transfer.second;
		char host1_lan = host_to_lan[host1_id], host2_lan = host_to_lan[host2_id];
		int time = global_time;

		for (auto const &bridge_id: lan_to_bridges[host1_lan]) {
			network_graph[bridge_id].AddData(Bridge::Data(time + 1, host1_id, host2_id, host1_lan));
		}
		time++;

		while (time - global_time < 100) {
			for (int bridge_num = 1; bridge_num <= num_of_bridges; bridge_num++) {
				network_graph[bridge_num].UpdateOnData(time);
			}
			for (int bridge_num = 1; bridge_num <= num_of_bridges; bridge_num++) {
				if (network_graph[bridge_num].IsDataPresent(time)){
					auto generated_data = network_graph[bridge_num].GenerateData(time);
					for (auto const &data: generated_data) {
						for (const int &receiver_bridge_num: lan_to_bridges[data.port_id]) {
							if (receiver_bridge_num != bridge_num) {
								network_graph[receiver_bridge_num].AddData(data);
							}
						}
					}
				}
			}
			time++;
		}
		for (int bridge_num = 1; bridge_num <= num_of_bridges; bridge_num++) {
			network_graph[bridge_num].PrintForwardingTable();
		}
		cout << endl;
	}
}

int main() {
	bool trace; cin >> trace;
	int num_of_bridges; cin >> num_of_bridges;

	vector<Bridge> network_graph;
	map< char, vector<int> > lan_to_bridges;
	ReadNetworkGraph(num_of_bridges, trace, network_graph, lan_to_bridges); // indexed from 1 to num_of_bridges
	int num_of_lans = lan_to_bridges.size();

	SpanningTreeProtocol(network_graph, lan_to_bridges);

	vector<char> host_to_lan; ReadHostToLan(num_of_lans, host_to_lan); // host_to_lan is 1-indexed
	int num_of_hosts = host_to_lan.size()-1;
	int num_of_data_transfers; cin >> num_of_data_transfers;
	vector< pair<int, int> > data_transfers; ReadDataTransfers(num_of_data_transfers, data_transfers);

	LearningBridges(network_graph, host_to_lan, data_transfers);
}
