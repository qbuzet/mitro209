#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <regex>
#include <climits>

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
    string file_name = "artist_edges";
    string ext = ".csv";
    string exp = ".txt";
    ifstream file(repertory + file_name + ext, ios::in);
    if(file){

        string out_file_name = file_name.append("_ri"); //ri = re indexed
        ofstream out_file(repertory + out_file_name + exp, ios::out | ios::trunc);
        if(out_file){
            int nbNodes = 0;
            int nbEdges = 0;

            //I can start reading the graph after this line
            regex reg("node_1,node_2");
            smatch sm;
            char line[256] = "";
            string s = line;
            while(!regex_search(s, sm, reg)) {
                file.getline(line,256,'\n');
                s = line;
            }

            //Start of reading the graph
            unordered_map<int, int> nodesMap;
            unordered_map<edge, bool, edge::hashFunction> edgesMap;
            int index = 0;
            int nbDuplicates = 0;
            while(!file.eof()){
                int node1;
                int node2;
                char sep;
                file >> node1 >> sep >> node2;

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
                nbEdges++;
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
