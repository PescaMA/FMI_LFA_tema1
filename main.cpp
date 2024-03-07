/// un cod LUNG (asa se intampla cand codez eu clase) care respecta toate cerintele si individuale,
/// implementand algoritmi relativ eficienti pe DFA, NFA, lambda-NFA.
/// (facut de Pescariu Matei-Alexandru - tema 1 LFA).
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <unordered_set>
#include <stack>
#include <deque>

class FA
{ /// abstract implementation of a finite automata
protected:
    int startState;
    std::unordered_set<int> finalStates; /// unordered_set for O(1) verification if a state is final.
    std::unordered_map<int, std::unordered_map<char,std::unordered_set<int> > > edges;
    /// edges is an adjacency list. [StartingNode] [Edge "cost"/letter] [EndingNode].
    /// Why so many unordered? For theoretical O(1) access. When using the list I go on transitions based on the
    /// letter more than based on the next node, so it comes before for faster access.
    /// The set is used for lambdaNFAs to search for the lambda transition. It also is compatible with DFA/NFA.

    virtual void path(std::string word) = 0;/// displays path in console. (on DFA) should be used
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
    FA(const FA& fa):startState(fa.startState),edges(fa.edges){}
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
    virtual bool accepted(std::string word, bool a = false) = 0; /// implemented in non-abstract FA.
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
    bool acceptedDFS(std::string word, int node,int i=0)
    {
        if((unsigned)i == word.size())
            return (finalStates.find(node) != finalStates.end());

        if(edges[node].find(word[i]) != edges[node].end())
            for(const auto& nextNode:edges[node][word[i]])
                if(acceptedDFS(word,nextNode,i+1))
                    return true;
        return false;
    }

    /// first coded. Doesn't output path (can't remember where all paths came from).
    bool acceptedBFS(std::string word)
    {
        if(word.size() == 0)
            return (finalStates.find(startState) != finalStates.end());
        std::unordered_set<int> nodes;
        nodes.insert(startState);
        for(unsigned int i = 0; i < word.size(); i++)
        {
            std::unordered_set<int> oldNodes = nodes;
            nodes.clear();
            for(const auto& node:oldNodes)
            {
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
};

/// the lambda non deterministic finite automata. The most important one, containg all NFAs or DFAs.
class l_NFA : public FA
{
protected:
    void path(std::string word)    {
        accepted(word,true);
    }

    /// helper function to get the lambda path between 2 nodes (used in printing path).
    std::deque<int> getLambdaPath(int prevNode, char c, int nextNode)
    {
        std::unordered_map<int,int> lambdaPath; /// has 2 uses: remembers what
        /// nodes have been visited (to ignore lambda cycles) and remembers from
        /// which node we got to the current node.
        lambdaPath[prevNode] = prevNode; /// to figure out it's the first one.

        std::stack<int> stk;
        stk.push(prevNode);
        while(!stk.empty())
        {
            int node = stk.top();
            if(edges[node][c].find(nextNode) != edges[node][c].end())
                break; /// if we found an edge to the desired next node we don't need to check any more nodes.
            stk.pop();

            for (const auto& sameNode:edges[node]['0'])
            { /// sameNode because we can get there through lambda transitions in the same step.
                if(lambdaPath.find(sameNode) != lambdaPath.end())
                    continue; /// if already visited we ignore it.
                stk.push(sameNode); /// we'll have to check it.
                lambdaPath[sameNode] =  node;
            }
        }
        if(stk.empty() || edges[stk.top()][c].find(nextNode) == edges[stk.top()][c].end())
            return {}; /// control case where no path was found or it doesn't lead correctly.

        /// backtracks to the beginning node, saving the nodes.
        int node = stk.top();
        std::deque<int> result;
        while(node != lambdaPath[node])
        {
            result.push_back(node);
            node = lambdaPath[node];
        }
        result.push_back(nextNode);
        return result;
    }

    void printPath(std::string word, std::deque<std::pair<int,int> >removedNodes)
    {
        int pathNodes[word.size()]; /// the correct path nodes (the last removed nodes per place
        /// in the word, because if an index was removed multiple times it means the path didn't
        /// lead to the end for all but the last one)

        for (const auto& node : removedNodes)
            pathNodes[node.second] = node.first; /// second = i, index of letter; first = node

        std::deque< int > result;
        result.push_back(pathNodes[0]); /// should be the start node.

        for(unsigned int i=1;i<=word.size();i++)
        {
            if(edges[result.back()][word[i-1]].find(pathNodes[i])
               != edges[result.back()][word[i-1]].end())
            { /// if there is a edge between previous node and current one
                result.push_back(pathNodes[i]);
                continue;
            }

            /// now we know we used lambdas to get to node[i] so we need to find it.
            std::deque<int> lambdaPath = getLambdaPath(result.back(),word[i-1],pathNodes[i]);

            while(!lambdaPath.empty())
            {
                result.push_back(lambdaPath.front());
                lambdaPath.pop_front();
            }
        }

        while(!result.empty())
        {
            std::cout << result.front() << ' ' ;
            result.pop_front();
        }
    }
    bool acceptEmpty()
    {
        if(finalStates.find(startState) != finalStates.end())
            return true; /// if the start node is also a final node.
        for(const auto& lambdaNode:edges[startState][0])
            if(finalStates.find(lambdaNode) != finalStates.end())
                return true;
        return false;
    }

public:

    using FA::runTests;
    bool accepted(std::string word, bool outputPath = false)
    {
        if(word.empty()) /// special case
            return acceptEmpty();

        std::stack<std::pair<int,int> > stk; /// remembers the nodes and the index of the added nodes
        /// to the iterative depth first search "simulated" stack
        stk.push({startState,0});

        std::deque<std::pair<int,int> >  removedNodes; /// remembers the nodes and the index of the popped nodes.
        ///`Used for rebuilding and printing the path.

        while(!stk.empty())
        {
            std::unordered_set<int> same,vis; /// "same" is for nodes reachable with a lambda path
            /// and "vis" for nodes that can be visited on the next
            int node = stk.top().first;
            unsigned int i = stk.top().second;

            removedNodes.push_back(stk.top());
            stk.pop();

            if(i == word.size())
            {/// reached the end of the word. Finish or backtrack
                if(finalStates.find(node) != finalStates.end())
                {
                    if(outputPath)
                        printPath(word, removedNodes);
                    return true;
                }

                continue;
            }

            std::stack<int> startNodes; /// nodes that can be reached by lambda paths (start next edge)
            startNodes.push(node);
            same.insert(node);

            /// calcualtes the set of nodes which we can begin from
            while(!startNodes.empty())
            {
                int sameNode = startNodes.top();
                startNodes.pop();
                /// add all new lambda edges to the set (and to be verified).
                for (const auto& nextNode:edges[node]['0'])
                {
                    if(same.find(nextNode) == same.end())
                    {
                        same.insert(nextNode);
                        startNodes.push(nextNode);
                    }
                }

                /// add the new edges to the next visitable set.
                for (const auto& nextNode:edges[sameNode][word[i]])
                    vis.insert(nextNode);
            }

            /// adds all the reachable nodes.
            for (const auto& nextNode:vis)
                stk.push({nextNode,i+1});
        }
        return false;
    }
};

int main()
{
    std::ifstream fin("input.txt");
    std::ofstream fout("output.txt");
    l_NFA automata;
    fin >> automata;
    automata.runTests(fin,fout);
}
