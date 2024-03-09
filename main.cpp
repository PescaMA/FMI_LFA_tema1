/// un cod FOARTE LUNG (asa se intampla cand codez eu clase + am incercat sa fie citibil +
/// probabil m-am complicat enorm) care respecta toate cerintele si individualizate,
/// implementand algoritmi relativ eficienti pe DFA, NFA, lambda-NFA.
/// (facut de Pescariu Matei-Alexandru - tema 1 LFA).
/// P.S. sper ca nu e vreo problema ca am pus comentariile in engleza ca sa fie mai universal, sa il pot pune usor pe GitHub.
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <unordered_set>
#include <stack>
#include <deque>

class FA
{ /// abstract implementation of a finite automata
protected:
    const char LAMBDA = '@';
    int startState;
    std::unordered_set<int> finalStates; /// unordered_set for O(1) verification if a state is final.
    std::unordered_map<int, std::unordered_map<char,std::unordered_set<int> > > edges;
    /// edges is an adjacency list. [StartingNode] [Edge "cost"/letter] [EndingNode].
    /// Why so many unordered? For theoretical O(1) access. When using the list I go on transitions based on the
    /// letter more than based on the next node, so it comes before for faster access.
    /// The set is used for lambdaNFAs to search for the lambda transition. It also is compatible with DFA/NFA.

    virtual void path(std::string word) {};/// displays path in console. MUST be implemented in derived classes.
    void runTests(std::istream& in,std::ostream& out)
    { /// without the implementation of accepted() and path() it doesn't work.
    /// Therefore, it only becomes public in derived classes.
        int testCount;
        in >> testCount;
        for(int i=0;i<testCount;i++)
        {
            std::string word;
            in >> word;
            if(word == "_") word = ""; /// Empty word is considered to be the underscore.
            if(accepted(word))
            {
                out << "DA\n"; /// means Yes (the word is accepted)
                std::cout << "Drumul este: "; /// means "The path is: "
                path(word);
                std::cout << '\n';
            }
            else
            {
                out << "NU\n"; /// means No
                std::cout << "Cuvantul nu e acceptat.\n"; /// means "The word isn't accepted."
            }
        }
    }
public:
    FA(){}
    FA(const FA& fa){set_FA(fa);}
    void set_FA(FA newFa){startState = newFa.startState; finalStates = newFa.finalStates; edges = newFa.edges;}
    FA get_FA(){
        FA result;
        result.startState = startState;
        result.finalStates = finalStates;
        result.edges = edges;
        return result;
    }
    void read(std::istream& in)
    {/// input data format is present in requirements.txt

        int n,m;
        std::deque<int> states;
        /// Didn't use these variables so I didn't put them as members.

        in >> n;
        for(int i=0;i<n;i++)
        {
            int state;
            in >> state;
            states.push_back(state);
        }
        in >> m;
        for(int i=0;i<m;i++)
        {
            int startNode, endNode;
            char letter;
            in >> startNode >> endNode >> letter;
            edges[startNode][letter].insert(endNode);
        }
        in >>startState;
        std::string listFinals;
        int nrFinalStates;
        in >> nrFinalStates;
        for(int i=0;i<nrFinalStates;i++)
        {
            int state;
            in >> state;
            finalStates.insert(state);
        }
    }
    virtual bool accepted(std::string word, bool a = false){return false;} /// MUST be implemented in non-abstract FA.
    friend std::istream& operator>>(std::istream& in, FA& myDFA);
};
std::istream& operator>>(std::istream& in, FA& myFA)
{
    myFA.read(in);
    return in;
}


class DFA : public FA
{
public:
    using FA::runTests; /// to make the runTests public.
    void path(std::string word){
        accepted(word,true);
    }
    bool accepted(std::string word,bool outputPath = false)
    { /// simple acceptor in O(L) where L is the length of the word.
        if(word.size() == 0)
            return (finalStates.find(startState) != finalStates.end());
        int node = startState;
        for(unsigned int i = 0; i < word.size(); i++)
        {
            if(edges[node].find(word[i]) == edges[node].end())
                return false;
            node = *(edges[node][word[i]].begin()); /// since it's a DFA we should only have 1 transition per letter per node.
        }
        return (finalStates.find(node) != finalStates.end());
    }
};


class NFA : public FA
{
public:
    using FA::runTests;
    void path(std::string word){
        accepted(word,true);
    }
    void printPath(std::deque<int> nodes, int lastNode)
    {
        while(!nodes.empty())
        {
            std:: cout << nodes.front() << ' ';
            nodes.pop_front();
        }
        std::cout << lastNode;
    }


    /// Slightly OVERCOMPLICATED (took some time to debug). works but not traditional DFS.
    /// It was the first solution to also output the path that I thought of.
    /// Uses a stack of iterators, which ended up needing the last node
    /// to verify that the end of the iterator isn't reached (luckily
    /// prevNodes is also used in outputing the result).

    /// It doesn't use a normal stack because I thought it would be too memory
    /// inefficient to have worst case of n states per every l letter stored instead of l iterators.
    /// At lambda-NFA I gave up on this design in favor of simplicity.
    bool accepted(std::string word,bool outputPath = false)
    {
        if(word.size() == 0) /// empty word (lambda)
            return (finalStates.find(startState) != finalStates.end());
        if(edges[startState].find(word[0]) == edges[startState].end())
            return false; /// if the first letter doesn't lead anywhere

        std::deque<int> prevNodes;
        prevNodes.push_back(startState);

        std::stack< std::unordered_set<int>::iterator > its;
        its.push(edges[startState][word[0]].begin());

        /// this for loop is just weird. i is the index of the place in the word
        /// and when going on a bad path we decrease it once (i-=2;i++). So it
        /// has to end when running out of every path (backtracking to 0th letter).
        for(int i=1;i >= 0; i++)
        {
            if(its.empty())return false;
            if(its.top() == edges[prevNodes.back()][word[i-1]].end())
            {/// if we reached the end of the current iterator.
                i-=2;
                its.pop();
                prevNodes.pop_back();
                if(!its.empty())
                    its.top()++;
                continue;
            }
            int node = *(its.top());

            if((unsigned)i == word.size())
            { /// reached the end of the word; testing if finalState.
                if (finalStates.find(node) != finalStates.end())
                {
                    if(outputPath)
                        printPath(prevNodes,node);
                    return true;
                }
                i-=1;
                its.top()++;
                continue;
            }


            if(edges[node].find(word[i]) == edges[node].end())
            {/// testing if transition doesn't exists
                i-=1;
                its.top()++;
                continue;
            }
            prevNodes.push_back(node);
            its.push(edges[node][word[i]].begin());
        }
        return false;
    }

    /// classic recursive DFS. Didn't know how to show path so I codded the overcomplicated one.
    bool acceptedDFS(std::string word, int node,int i=0){
        if((unsigned)i == word.size())
            return (finalStates.find(node) != finalStates.end());

        if(edges[node].find(word[i]) != edges[node].end())
            for(const auto& nextNode:edges[node][word[i]])
                if(acceptedDFS(word,nextNode,i+1))
                    return true;
        return false;
    }

    /// first coded. Doesn't output path (can't remember where all paths came from).
    bool acceptedBFS(std::string word){
        if(word.size() == 0)
            return (finalStates.find(startState) != finalStates.end());
        std::unordered_set<int> nodes;
        nodes.insert(startState);
        for(unsigned int i = 0; i < word.size(); i++){
            std::unordered_set<int> oldNodes = nodes;
            nodes.clear();
            for(const auto& node:oldNodes){
                if(edges[node].find(word[i]) != edges[node].end())
                    for(const auto& nextNode:edges[node][word[i]])
                        nodes.insert(nextNode);
            }
            if(nodes.empty()) return false;
        }
        for (const auto& node: nodes)
            if(finalStates.find(node) != finalStates.end())
                return true;
        return false;
    }
    DFA to_DFA();
};

/// the lambda non deterministic finite automata. The most important one, containg all NFAs or DFAs.
class l_NFA : public FA
{
protected:
    void path(std::string word)    {
        accepted(word,true);
    }
    bool checkTransition(int prevNode,char c,int nextNode){
        return edges.find(prevNode) != edges.end() &&
               edges[prevNode].find(c) != edges[prevNode].end() &&
               edges[prevNode][c].find(nextNode) != edges[prevNode][c].end();
    }
    int getLambdaLast(int prevNode,char c,int nextNode){
        std::unordered_set<int> same = getReachableLambda(prevNode);
        for(const auto& sameNode:same)
        {
            if(!checkTransition(sameNode,c,nextNode))
                continue;
            return sameNode;
        }
        return prevNode;
    }
    std::unordered_map<int,int> getLambdaPrevious(int startNode){
        std::unordered_map<int,int> lambdaPath; /// has 2 uses: remembers what
        /// nodes have been visited (to ignore lambda cycles) and remembers from
        /// which node we got to the current node.
        lambdaPath[startNode] = startNode; /// to figure out it's the first one.

        std::stack<int> stk;
        stk.push(startNode);
        while(!stk.empty())
        {
            int node = stk.top();
            stk.pop();

            for (const auto& sameNode:edges[node][LAMBDA])
            { /// sameNode because we can get there through lambda transitions in the same step.
                if(lambdaPath.find(sameNode) != lambdaPath.end())
                    continue; /// if already visited we ignore it.
                stk.push(sameNode); /// we'll have to check it.
                lambdaPath[sameNode] =  node;
            }
        }
        return lambdaPath;
    }
    std::deque<int> getLambdaPath(int startNode,int endNode){
        if(startNode == endNode)
            return {};
        std::unordered_map<int,int> lambdaPath = getLambdaPrevious(startNode);
        /// backtracks to the beginning node, saving the nodes.

        std::deque<int> result;
        while(endNode != lambdaPath[endNode])
        {
            result.push_front(endNode);
            endNode = lambdaPath[endNode];
        }
        return result;
    }
    /// helper function to get the lambda path between 2 nodes (used in printing path).
    std::deque<int> getLambdaTransitionPath(int prevNode, char c, int nextNode){

        int lastNode = getLambdaLast(prevNode,c,nextNode);

        if(!checkTransition(lastNode,c,nextNode))
            return {}; /// control case where no path was found

        std::deque<int> result = getLambdaPath(prevNode,lastNode);
        return result;
    }
    void printPath(std::string word, std::deque<std::pair<int,int> >removedNodes){
        int pathNodes[word.size()]; /// the correct path nodes (the last removed nodes per place
        /// in the word, because if an index was removed multiple times it means the path didn't
        /// lead to the end for all but the last one)

        for (const auto& node : removedNodes)
            pathNodes[node.second] = node.first; /// second = i, index of letter; first = node

        std::deque< std::pair<int,char> > result;
        result.emplace_back(pathNodes[0],' '); /// should be the start node.

        for(unsigned int i=1;i<=word.size();i++)
        {
            int node = result.back().first;

            if(edges[node][word[i-1]].find(pathNodes[i]) == edges[node][word[i-1]].end()){
                std::deque<int> lambdaPath = getLambdaTransitionPath(node,word[i-1],pathNodes[i]);
                for(const auto& lambdaNode:lambdaPath)
                    result.emplace_back(lambdaNode,'@');
            }
            result.emplace_back(pathNodes[i],word[i-1]);
        }
        int node = result.back().first;
        std::deque<int> lambdaPath = getLambdaPath(node,getFinishNode(node));
        for(const auto& lambdaNode:lambdaPath)
                result.emplace_back(lambdaNode,'@');

        while(!result.empty())
        {
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
    std::unordered_set<int> getReachableLambda(int node){
        std::unordered_set<int> same;
        std::stack<int> startNodes; /// nodes that can be reached by lambda paths (start next edge)
        startNodes.push(node);
        same.insert(node);

        /// calcualtes the set of nodes which we can begin from
        while(!startNodes.empty())
        {
            int sameNode = startNodes.top();
            startNodes.pop();
            /// add all new lambda edges to the set (and to be verified).
            for (const auto& nextNode:edges[sameNode][LAMBDA])
            {
                if(same.find(nextNode) == same.end())
                {
                    same.insert(nextNode);
                    startNodes.push(nextNode);
                }
            }
        }
        return same;
    }
    bool acceptedEmpty(bool outputPath = false){
        if(!isFinishReachable(startState))
            return false;
        if(!outputPath)
            return true;
        std::deque<int> path = getLambdaPath(startState,getFinishNode(startState));
        if(finalStates.find(startState) != finalStates.end())
            path.push_back(startState);
        while(!path.empty())
        {
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
        ///std::unordered_set<int> visited[word.size() + 2]; // NEW

        if(word.empty()) /// special case
            return acceptedEmpty(outputPath);

        std::stack<std::pair<int,int> > stk; /// remembers the nodes and the index of the added nodes
        /// to the iterative depth first search "simulated" stack
        stk.push({startState,0});

        std::deque<std::pair<int,int> >  removedNodes; /// remembers the nodes and the index of the popped nodes.
        ///`Used for rebuilding and printing the path.

        while(!stk.empty())
        {

            int node = stk.top().first;
            unsigned int i = stk.top().second;
            removedNodes.push_back(stk.top());
            stk.pop();
            ///std::cout <<"nod/i: " <<  node << ' ' << i << '\n';

            ///  if(visited[i].find(node) != visited[i].end()) // NEW
            ///    continue; // NEW
            ///visited[i].insert(node);

            if(i == word.size())
            {/// reached the end of the word. Finish or backtrack
                if(isFinishReachable(node))
                {
                    if(outputPath)
                        printPath(word, removedNodes);
                    return true;
                }

                continue;
            }

            std::unordered_set<int> same,vis; /// "same" is for nodes reachable with a lambda path
            /// and "vis" for nodes that can be visited on the next
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
    DFA to_DFA();
};

NFA l_NFA::to_NFA()
{
    FA original;
    original.set_FA(this->get_FA());

    std::deque<int> nodes;
    for(const auto& edge:edges)
        nodes.push_back(edge.first);

    for(const auto& node:nodes)
    {
        if(edges.find(node) == edges.end())continue;

        if(edges[node].find(LAMBDA) == edges[node].end())
            continue;
        if(edges[node][LAMBDA].empty()) continue;

        edges[node][LAMBDA].erase(node);


        while(!edges[node][LAMBDA].empty())
        { /// we can't use a for because the iterated vector might change.
            int nextNode = *(edges[node][LAMBDA].begin());
            if(finalStates.find(nextNode) != finalStates.end())
                finalStates.insert(node);
            for(const auto& edge:edges[nextNode])
            {
                for(const auto& newNode:edge.second)
                    edges[node][edge.first].insert(newNode);
            }
            edges[node][LAMBDA].erase(nextNode);
            edges[node][LAMBDA].erase(node);
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
    ///NFA convNFA = automata.to_NFA();
    automata.runTests(fin,fout);
}
