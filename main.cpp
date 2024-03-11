/// un cod FOARTE LUNG care respecta toate cerintele si individualizate,
/// implementand algoritmi relativ eficienti pe DFA, NFA, lambda-NFA.
/// (facut de Pescariu Matei-Alexandru - tema 1 LFA).
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <unordered_set>
#include <stack>
#include <deque>
#include <vector>

class FA{ /// abstract implementation of a finite automata
protected:
    const char LAMBDA = '@'; /// the character decided to be lambda/epsilon/empty symbol.
    int startState;
    std::unordered_set<int> finalStates; /// unordered_set for O(1) verification if a state is final.
    std::unordered_map<int, std::unordered_map<char,std::unordered_set<int> > > edges;
    /// edges is an adjacency list. [StartingNode] [Edge "cost"/letter] [EndingNode].
    /// Why so many unordered? For theoretical O(1) access. When using the list I go on transitions based on the
    /// letter more than based on the next node, so it comes before for faster access.
    /// The set<int> is used for lambdaNFAs to search for the lambda transition. It also is compatible with DFA/NFA.

    virtual void path(std::string word) {}/// displays path in console. MUST be implemented in derived classes.
    void runTests(std::istream& in,std::ostream& out){ /// without the implementation of accepted() and path() it doesn't work.
    /// Therefore, it only becomes public in derived classes.
        int testCount;
        in >> testCount;
        for(int i=0;i<testCount;i++){
            std::string word;
            in >> word;
            if(word == "_") word = ""; /// Empty word is considered to be the underscore.
            if(accepted(word)){
                out << "DA\n"; /// means Yes (the word is accepted)
                std::cout << "Drumul este: "; /// means "The path is: "
                path(word);
                std::cout << '\n';
            }
            else{
                out << "NU\n"; /// means No
                std::cout << "Cuvantul nu e acceptat.\n"; /// means "The word isn't accepted."
            }
        }
    }
public:
    FA(){}
    FA(const FA& fa){set_FA(fa);}
    void set_FA(FA newFa){startState = newFa.startState; finalStates = newFa.finalStates; edges = newFa.edges;}
    FA get_FA(){ /// gets base common variables (needed in derived classes).
        FA result;
        result.startState = startState;
        result.finalStates = finalStates;
        result.edges = edges;
        return result;
    }
    void read(std::istream& in){/// input data format is present in requirements.txt
        int n,m;
        std::deque<int> states;
        std::string listFinals;
        int nrFinalStates;
        /// Didn't use these variables so I didn't put them as members.
        in >> n;
        for(int i=0;i<n;i++){
            int state;
            in >> state;
            states.push_back(state);
        }
        in >> m;
        for(int i=0;i<m;i++){
            int startNode, endNode;
            char letter;
            in >> startNode >> endNode >> letter;
            edges[startNode][letter].insert(endNode);
        }
        in >>startState;
        in >> nrFinalStates;
        for(int i=0;i<nrFinalStates;i++){
            int state;
            in >> state;
            finalStates.insert(state);
        }
    }
    virtual bool accepted(std::string word, bool a = false){return false;} /// MUST be implemented in non-abstract FA.
    friend std::istream& operator>>(std::istream& in, FA& myDFA);
};
std::istream& operator>>(std::istream& in, FA& myFA){
    myFA.read(in);
    return in; /// used read so I could write that inside the class.
}
class DFA : public FA{
public:
    using FA::runTests; /// to make the runTests public.
    void path(std::string word){
        accepted(word,true);
    }
    bool accepted(std::string word,bool outputPath = false){ /// simple acceptor in O(L) where L is the length of the word.
        if(word.size() == 0)
            return (finalStates.find(startState) != finalStates.end());
        int node = startState;
        for(unsigned int i = 0; i < word.size(); i++){
            if(edges[node].find(word[i]) == edges[node].end())
                return false;
            node = *(edges[node][word[i]].begin()); /// since it's a DFA we should only have 1 transition per letter per node.
        }
        return (finalStates.find(node) != finalStates.end());
    }
};
class NFA : public FA{
public:
    using FA::runTests;
    void path(std::string word){
        accepted(word,true);
    }
    void printPath(const std::vector<int>& nodes, std::string word){
        for(int unsigned i = 0;i < word.size(); i++)
            std:: cout << nodes[i] << " -" << word[i] << "-> ";
        std::cout << nodes[word.size()];
    }
    bool acceptedEmpty(bool outputPath = false){ /// handles empty word
        if(finalStates.find(startState) == finalStates.end())
            return false;
        if(outputPath)
            std::cout << startState;
        return true;
    }
    bool accepted(std::string word, bool outputPath = false){
        if(word.empty()) /// special case
            return acceptedEmpty(outputPath);

        std::unordered_set<int> visited[word.size() + 2]; /// which nodes have been visited for each letter to
        /// not compute the same thing multiple times. Costs O(word.size * nr_states) space worst case.

        std::vector<int> solutionNodes(word.size()+2);
        std::stack<std::pair<int,int> > stk; /// remembers the nodes and the index of the added nodes
        /// to the iterative depth first search "simulated" stack
        stk.push({startState,0});

        while(!stk.empty()){
            int node = stk.top().first;
            unsigned int i = stk.top().second;
            stk.pop();

            solutionNodes[i] = node; /// position i will be rewritten if dfs backtracks.

            if(visited[i].find(node) != visited[i].end())
                continue;
            visited[i].insert(node);

            if(i == word.size()){/// reached the end of the word. Finish or backtrack
                if(finalStates.find(node) == finalStates.end())
                    continue;
                if(outputPath)
                    printPath(solutionNodes,word);
                return true;
            }

            for (const auto& nextNode:edges[node][word[i]])
                stk.push({nextNode,i+1});
        }
        return false;
    }
};
class l_NFA : public FA{ /// the lambda non deterministic finite automata. The most important one, NFAs and DFAs being special cases of this.
protected:
    void path(std::string word){ accepted(word,true); }
    bool checkTransition(int prevNode,char c,int nextNode){ /// checks if transition exists.
        return edges.find(prevNode) != edges.end() &&
               edges[prevNode].find(c) != edges[prevNode].end() &&
               edges[prevNode][c].find(nextNode) != edges[prevNode][c].end();
    }
    int getLambdaLast(int prevNode,char c,int nextNode){ /// gets a lambda reachable node that has a transition c to nextNode.
        std::unordered_set<int> same = getReachableLambda(prevNode);
        for(const auto& sameNode:same){
            if(!checkTransition(sameNode,c,nextNode))
                continue;
            return sameNode;
        }
        return prevNode;
    }
    std::unordered_map<int,int> getLambdaPrevious(int startNode){ /// allows computation of lambda paths by saying which
        /// node we reached the current node from.

        std::unordered_map<int,int> lambdaPath; /// has 2 uses: remembers what nodes have been visited (to ignore
        ///lambda cycles) and remembers from which node we got to the current node.
        lambdaPath[startNode] = startNode; /// to figure out it's the first one.

        std::stack<int> stk;
        stk.push(startNode);
        while(!stk.empty()){
            int node = stk.top();
            stk.pop();

            for (const auto& sameNode:edges[node][LAMBDA]){ /// sameNode because we can get there through lambda transitions in the same step.
                if(lambdaPath.find(sameNode) != lambdaPath.end())
                    continue; /// if already visited we ignore it.
                stk.push(sameNode); /// we'll have to check it.
                lambdaPath[sameNode] =  node; /// reached it from the node called node.
            }
        }
        return lambdaPath;
    }
    std::deque<int> getLambdaPath(int startNode,int endNode){/// gets a path between a node and a reachable lambda
        if(startNode == endNode) return {};
        std::unordered_map<int,int> lambdaPath = getLambdaPrevious(startNode);

        std::deque<int> result;
        while(endNode != lambdaPath[endNode]){ /// backtracks path from end to start.
            result.push_front(endNode);
            endNode = lambdaPath[endNode];
        }
        return result;
    }

    std::deque<int> getLambdaTransitionPath(int prevNode, char c, int nextNode){ /// gets a path to a node that passes through lambda transitions

        int lastNode = getLambdaLast(prevNode,c,nextNode);
        if(!checkTransition(lastNode,c,nextNode))
            return {}; /// control case where no path was found

        std::deque<int> result = getLambdaPath(prevNode,lastNode);
        return result;
    }
    std::unordered_set<int> getReachableLambda(int node){
        std::unordered_set<int> same;
        auto reachableFrom = getLambdaPrevious(node);
        for(const auto& sameNode:reachableFrom)
            same.insert(sameNode.first);
        return same;
    }
    void printPath(std::string word, std::deque<std::pair<int,int> >removedNodes){
        int pathNodes[word.size() + 2]; /// the correct path nodes (the last removed nodes per placein the word,
        /// because if an index was removed multiple times it means the path didn't lead to the end for all but the last one)

        for (const auto& node : removedNodes)
            pathNodes[node.second] = node.first; /// second = i, index of letter; first = node

        std::deque< std::pair<int,char> > result; /// contains node and transition to get there.
        result.emplace_back(pathNodes[0],' '); /// should be the start node.

        for(unsigned int i=1;i<=word.size();i++){ /// will add the lambda transitions when there isn't a direct path
            int node = result.back().first;

            if(checkTransition(node,word[i-1],pathNodes[i])){
                std::deque<int> lambdaPath = getLambdaTransitionPath(node,word[i-1],pathNodes[i]);
                for(const auto& lambdaNode:lambdaPath)
                    result.emplace_back(lambdaNode,LAMBDA);
            }
            result.emplace_back(pathNodes[i],word[i-1]);
        }
        /// if last node isn't a final state, add lambda transitions to final state
        int node = result.back().first;
        if(!isFinishReachable(node))
            throw std::logic_error("NoPathFound");

        std::deque<int> lambdaPath = getLambdaPath(node,getFinishNode(node));
        for(const auto& lambdaNode:lambdaPath)
            result.emplace_back(lambdaNode,LAMBDA);

        printWithSymbol(result);
    }
    void printWithSymbol(std::deque<std::pair<int,char> > result){
        while(!result.empty()){
            std::cout << result.front().first << ' ' ;
            result.pop_front();
            if(!result.empty())
                std::cout << "-" << result.front().second << "-> ";
        }
    }
    bool isFinishReachable(int node){
        return (finalStates.find(getFinishNode(node)) != finalStates.end());
    }
    int getFinishNode(int node){
        if(finalStates.find(node) != finalStates.end())
            return node;
        std::unordered_set<int> same = getReachableLambda(node);
        for(const auto& lambdaNode:same)
            if(finalStates.find(lambdaNode) != finalStates.end())
                return lambdaNode;
        return node;
    }

    bool acceptedEmpty(bool outputPath = false){
        if(!isFinishReachable(startState))
            return false;
        if(!outputPath)
            return true;

        std::deque<int> path = getLambdaPath(startState,getFinishNode(startState));
        if(finalStates.find(startState) != finalStates.end())
            path.push_back(startState);
        while(!path.empty()){
            std::cout << path.front();
            path.pop_front();
            if(!path.empty())
                std::cout << " -@-> ";
        }
        return true;
    }
public:
    using FA::runTests;
    bool accepted(std::string word, bool outputPath = false){
        std::unordered_set<int> visited[word.size() + 2];

        if(word.empty()) /// special case
            return acceptedEmpty(outputPath);

        std::stack<std::pair<int,int> > stk; /// remembers the nodes and the index of the added nodes to the iterative depth first search "simulated" stack
        stk.push({startState,0});

        std::deque<std::pair<int,int> >  removedNodes; /// remembers the nodes and the index of the popped nodes. Used for rebuilding and printing the path.

        while(!stk.empty())
        {
            int node = stk.top().first;
            unsigned int i = stk.top().second;
            removedNodes.push_back(stk.top());
            stk.pop();

            if(visited[i].find(node) != visited[i].end())
                continue;
            visited[i].insert(node);

            if(i == word.size())
            {/// reached the end of the word. Finish or backtrack
                if(!isFinishReachable(node))
                    continue; /// don't add next transitions.
                if(outputPath)
                    printPath(word, removedNodes);
                return true;
            }

            std::unordered_set<int> same,vis; /// "same" is for nodes reachable with a lambda path
            /// and "vis" for nodes that CAN be visited on the next
            same = getReachableLambda(node);

            for( const auto& sameNode:same)
                for(const auto& nextNode:edges[sameNode][word[i]])
                    vis.insert(nextNode);/// add the new edges to the next visitable set.

            /// adds all the reachable nodes.
            for (const auto& nextNode:vis)
                stk.push({nextNode,i+1});
        }
        return false;
    }
    NFA to_NFA();
};
NFA l_NFA::to_NFA()
{
    FA original; /// keep old data to copy it back.
    original.set_FA(this->get_FA());

    std::deque<int> nodes; /// all nodes(states) that have transitions
    for(const auto& edge:edges)
        nodes.push_back(edge.first);

    for(const auto& node:nodes)
    {
        std::unordered_set<int> reachable = getReachableLambda(node);
        edges[node][LAMBDA].clear();

        for(const auto& sameNode:reachable){
            if(finalStates.find(sameNode) != finalStates.end())
                finalStates.insert(node);
            for(const auto& edge:edges[sameNode])
            {
                if(edge.first == LAMBDA)
                    continue;
                for(const auto& newNode:edge.second)
                    edges[node][edge.first].insert(newNode);
            }
        }
    }

    NFA result;
    result.set_FA(this->get_FA());
    this->set_FA(original.get_FA());
    return result;
}
int main()
{
    std::ifstream fin("input.txt");
    std::ofstream fout("output.txt");
    l_NFA automata;
    fin >> automata;
    NFA convNFA = automata.to_NFA();
    convNFA.runTests(fin,fout);
}
