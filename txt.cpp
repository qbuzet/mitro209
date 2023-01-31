#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <regex>

using namespace std;

//Usually not needed
const bool ADD_MISSING_EDGES = false;
const bool REMOVE_DUPLICATES = false;

typedef struct edge {
    int node1;
    int node2;

    edge(int node1, int node2){
        this->node1 = node1;
        this->node2 = node2;
    }

    bool operator==(const edge& oe) const
    {
        return (this->node1 == oe.node1 && this->node2 == oe.node2);
    }

    struct hashFunction //It's necessary to define a hash function which can operate on pair
    {
        size_t operator()(const edge& e) const
        {
            size_t node1Hash = std::hash<int>()(e.node1);
            size_t node2Hash = std::hash<int>()(e.node2) << 1;
            return node1Hash ^ node2Hash;
        }
    };

} edge;

int main(int argc, char** argv){
    string repertory = "data/";
    string file_name = "roadNet-CA";
    string ext = ".txt";
    ifstream file(repertory + file_name + ext, ios::in);
    if(file){

        string out_file_name = file_name.append("_ri"); //ri = re indexed
        ofstream out_file(repertory + out_file_name + ext, ios::out | ios::trunc);
        if(out_file){
            int nbNodes;
            int nbEdges;

            //I'm searching the number of nodes and edges in the input file
            regex reg("Nodes: ([0-9]+) Edges: ([0-9]+)");
            smatch sm;
            char line[256] = "";
            string s = line;
            while(!regex_search(s, sm, reg)) {
                file.getline(line,256,'\n');
                s = line;
            }

            nbNodes = stoi(sm[1]);
            nbEdges = stoi(sm[2]);

            out_file << nbNodes << " " << nbEdges << endl;

            //I can start reading the graph after this line
            regex reg2("# FromNodeId	ToNodeId");
            smatch sm2;
            char line2[256] = "";
            string s2 = line2;
            while(!regex_search(s2, sm2, reg2)) {
                file.getline(line2,256,'\n');
                s2 = line2;
            }

            //Start of reading the graph
            unordered_map<int, int> nodesMap;
            unordered_map<edge, bool, edge::hashFunction> edgesMap;
            int index = 0;
            int nbDuplicates = 0;
            for(int i = 0; i < nbEdges; i++){
                int node1;
                int node2;
                file >> node1 >> node2;

                if(nodesMap.find(node1) == nodesMap.end()){
                    nodesMap.insert({node1, index});
                    index++;
                }
                if(nodesMap.find(node2) == nodesMap.end()){
                    nodesMap.insert({node2, index});
                    index++;
                }

                int index1 = nodesMap[node1];
                int index2 = nodesMap[node2];

                edge e = edge(index1, index2);
                if(REMOVE_DUPLICATES){
                    edge duedge = edge(index2, index1);
                    if(edgesMap.find(duedge) != edgesMap.end()){
                        nbDuplicates++;
                        continue;
                    }
                }

                edgesMap.insert({e, true});
                out_file << index1 << " " << index2 << endl;
            }

            int nbAddedEdges = 0;
            if(ADD_MISSING_EDGES){
                for(auto a : edgesMap){
                    edge b = edge(a.first.node2, a.first.node1);
                    if(edgesMap.find(b) == edgesMap.end()){
                        edgesMap.insert({b, true});
                        nbAddedEdges++;
                    }
                }
                nbEdges += nbAddedEdges;
            }

            if(REMOVE_DUPLICATES){
                nbEdges -= nbDuplicates;
            }

            nbNodes = index;
            out_file.seekp(0, ios::beg);
            out_file << nbNodes << " " << nbEdges << endl;   

            for(auto a : edgesMap){
                out_file << a.first.node1 << " " << a.first.node2 << endl;
            }

            cout << "nbNodes: " << nbNodes << " nbEdges: " << nbEdges << endl;

            if(ADD_MISSING_EDGES){
                cout << "Missing edges: " << nbAddedEdges << endl;
            }
            if(REMOVE_DUPLICATES){
                cout << "Duplacates: " << nbDuplicates << endl;
            }

            file.close();

        } else {
            cerr << "Unable to open the out file!" << endl;
        }

    } else {
        cerr << "Unable to open the file!" << endl;
    }

    return 0;
}