#include <iostream>
#include <fstream>
#include <vector>
#include <list>
#include <climits> 
#include <ctime>

using namespace std;

const bool PRINT_GRAPH = false; //Use this option to print the 2-approximation of the denset subgraph
const bool PRINT_NODES = false; //Use this option to print the list of nodes of the 2-approximation of the denset subgraph

typedef struct results {
    int nbNodes;
    int nbEdges;
    int approxNbNodes;
    int approxNbEdges;
    double density;
    double duration;

    results(int nbNodes, int nbEdges, int approxNbNodes, int approxNbEdges, double density, double duration){
        this->nbNodes = nbNodes;
        this->nbEdges = nbEdges;
        this->approxNbNodes = approxNbNodes;
        this->approxNbEdges = approxNbEdges;
        this->density = density;
        this->duration = duration;
    }
} results;

results algorithm(string file_name);

/*
 * Use this option if your graph is given with the edge (1,2) but also (2,1) for instance
 * If this constant is equal to false, the edge (2,1) will be added by the program
 */
bool ALREADY_UNDIRECTED = false;

/*
 * This program compute the 2-approximation of the denset subgraph
 */
int main(int argc, char** argv){
    /*
     * To generate a graph with the appropriate indexation, please first use txt.cpp
     * You can use .csv graph by using csv.cpp instead of txt.cpp
     *
     * You can find the graph I used to make my tests and benchmarks in /test and /data
     * For all the graph, please consider ALREADY_UNDIRECTED = false if nothing is specified
     * 
     * I use the following graphs:
     * /test => graph.txt
     *       => test1.txt
     *       => test2.txt
     *       => test3.txt
     *
     * /data => ca-AstroPh_ri.txt require ALREADY_UNDIRECTED = true
     *       => ca-HepPh_ri.txt, require ALREADY_UNDIRECTED = true
     *       => com-amazon.ungraph_ri.txt
     *       => com-dblp.ungraph_ri.txt
     *       => as20000102_ri.txt
     *       => HR_edges_ri.txt
     *       => HU_edges_ri.txt
     *       => RO_edges_ri.txt
     *       => web-BerkStan_ri.txt require ALREADY_UNDIRECTED = true
     *       => roadNet-CA require ALREADY_UNDIRECTED = true
     *       => large_twitch_edges_ri.txt
     * 
     * The program will perform on the graphs listed in graphs.txt
     * The first line indicates the number of graphs
     * The others are the paths to the files containing the graphs
     * Please indicate after the path if the graph is ALREADY_UNDIRECTED or not
     */

    ifstream graphs_file("graphs.txt", ios::in);
    if(graphs_file){

        ofstream results_file("results/results.csv", ios::out | ios::trunc);
        if(results_file){
            cout << "Début du traitement des graphes" << endl;
            results_file << "name,nbNodes,nbEdges,approxNbNodes,approxNbEdges,density,duration" << endl;
            int nbGraphs;
            graphs_file >> nbGraphs;
            for(int i = 0; i < nbGraphs; i++){
                string file_name;
                graphs_file >> file_name >> ALREADY_UNDIRECTED;
                results r = algorithm(file_name);
                results_file << file_name << "," << r.nbNodes << "," << r.nbEdges << ","<< r.approxNbNodes << "," << r.approxNbEdges << "," << r.density << "," << r.duration << endl;
            }
            graphs_file.close();
            cout << "Fin du traitement des graphes" << endl;
        } else {
            cout << "Unable to open results.txt" << endl;
        }
    } else {
        cout << "Unable to open graphs.txt" << endl;
    }

    return 0;
}


results algorithm(string file_name){
    //Reading the file
    results r = results(-1,-1,0,0,0,0);
    ifstream file(file_name, ios::in); //Be careful, check ALREADY_UNDIRECTED is correctly define
    if(file){
        int nbNodes;
        int nbEdges;

        file >> nbNodes >> nbEdges;

        vector<list<int>> adjList(nbNodes);
        for(int i = 0; i < nbEdges; i++){
            int node1;
            int node2;
            file >> node1 >> node2;
            adjList[node1].push_back(node2);
            if(!ALREADY_UNDIRECTED){
                adjList[node2].push_back(node1);
            }
        }

        if(ALREADY_UNDIRECTED){ //If the graph is given with (1,2) but also with (2,1) for instance, it means that nbEdges is twice the course definition
            nbEdges /= 2;
        }

        file.close();
        //End of reading the file
        clock_t start;
        clock_t end;
        double duration;
        start = clock();

        //Computation of the degree table
        vector<list<int>> nodesOfDegree (nbNodes + 1); //For each degree in [0,n], I maintain a list with all the nodes with this degree
        vector<int> degreeOfNode(nbNodes + 1); //It will be useful, to be able to know the degree of a given node
        vector<list<int>::iterator> posNode(nbNodes); //and to know is position in the corresponding list of degree to be able to suppress it instantly
        int minDegree = INT_MAX; //At the same time, I look for the minimum degree
        for(int i = 0; i < nbNodes; i++){
            int degree = adjList[i].size(); //The degree of the node is the number of edges linked to this node
            nodesOfDegree[degree].push_back(i); //I add the node in the list of corresponding degree
            degreeOfNode[i] = degree; //I store its degree
            posNode[i] = --(nodesOfDegree[degree].end()); //and its position

            if(minDegree > degree){ //I update the minimum degree
                minDegree = degree;
            }
        }
        //End of the computation of the degree table

        //Start of the algorithm
        //Note that I unalter the graph during the algorithm
        //I "simulate" the suppressions of nodes and edges with the degree table

        vector<int> trace; //I store the deletion order of the nodes, it will allow us to rebuild the graph at the end

        double approxDensity = (double) nbEdges/nbNodes; //Variables of the denset subgraph
        int approxNbNodes = nbNodes;
        int approxNbEdges = nbEdges;
        double currentDensity = (double) nbEdges/nbNodes; //Variables of the graph consider at each step of the algorithm
        int currentNbNodes = nbNodes;
        int currentNbEdges = nbEdges;

        for(int i = 0; i < nbNodes - 1; i++){
            //At each turn of the loop, I want to remove the node of minimum degree
            //When I remove a node I have to update the degree of nodes linked to it
            //Their degree can decrease only by 1
            //That's why, I can take minDegree - 1 and I don't need to recompute it entirely
            //If there's no node with such a degree, it compute the next one until we find a node
            minDegree = (minDegree - 1 < 0) ? 0 : minDegree - 1; //Minimum degree can't be below 0
            while (nodesOfDegree[minDegree].empty()){
                minDegree++;
                if(minDegree > nbNodes){  //This case isn't meant to happen
                    cerr << "Error: minDegree > nbNodes" << endl;
                    return r;
                }
            }

            int processNode  = nodesOfDegree[minDegree].back(); //The node I will remove
            trace.push_back(processNode);
            currentNbNodes -= 1;
            currentNbEdges -= minDegree; //There is minDegree edges starting from this node that I will remove
            nodesOfDegree[minDegree].pop_back(); //I remove the node
            degreeOfNode[processNode] = -1; //When I remove a node from the graph, I consider its degree equal to -1 to differentiate it from the node of degree 0

            //Now I have to process all the edges starting from this node
            for(int dest : adjList[processNode]){
                //If the destination node is still in the current subgraph
                if (degreeOfNode[dest] > 0) {
                    //As I suppressed processNode, the degree of the destination node is decreasing by 1
                    //I remove the destination node from its degree list to put it in its degree - 1 list 
                    nodesOfDegree[degreeOfNode[dest]].erase(posNode[dest]); //I remove it 
                    degreeOfNode[dest] -= 1; //I update its degree
                    nodesOfDegree[degreeOfNode[dest]].push_back(dest); //I put it in the new list
                    posNode[dest] = --(nodesOfDegree[degreeOfNode[dest]].end()); //I update its position
                }
            }

            //If the current subgraph have a better density than the best one already known, I update the variables of the approximation
            currentDensity = (double) currentNbEdges / currentNbNodes;
            if(currentDensity > approxDensity){
                approxDensity = currentDensity;
                approxNbNodes = currentNbNodes;
                approxNbEdges = currentNbEdges;
            }
        }

        end = clock();
        duration = (double) (end - start) / CLOCKS_PER_SEC;
        //End of the algorithm

        //Display of results
        cout << "Results for the graph: " << file_name << endl;
        cout << "nbNodes: " << approxNbNodes << " nbEdges: " << approxNbEdges << endl;
        cout << "Density: " << approxDensity << endl;
        cout << "Duration: " << duration << endl;

        //I create a table to know if a node is removed in the approximation or not
        int nbNodesToSupress = nbNodes - approxNbNodes; //I remove all the nodes of the first iterations to get the correct number of nodes in the approximation
        vector<bool> suppressedNodes(nbNodes);
        for (int i = 0; i < nbNodesToSupress; i++) {
            suppressedNodes[trace[i]] = true;
        }

        if(PRINT_GRAPH){
            cout << "2-approximation denset subgraph:" << endl;
            for (int i = 0; i < nbNodes; i++) {
                if (!suppressedNodes[i]){
                    for (int j : adjList[i]) {
                        if (!suppressedNodes[j]) {
                            cout << i << " " << j << endl; //I print the existing edges between to none suppressed nodes
                        }
                    }
                }
            }
        }

        if(PRINT_NODES){
            cout << "Remaining nodes: " << endl;
            for (int i = 0; i < nbNodes; i++) {
                if(!suppressedNodes[i]){ 
                    cout << i << " "; //I print the node not suppressed
                }
            }
            cout << endl;
        }
        //End of results display
    
        r = results(nbNodes, nbEdges, approxNbNodes, approxNbEdges, approxDensity, duration);

    } else {
        cerr << "Unable to open " << file_name << " !" << endl;
    }
    
    return r;
}