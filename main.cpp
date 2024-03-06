#include <iostream>
#include <fstream>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <algorithm>
#include <stack>
#include <deque>

class FA
{
protected:
    int n,m; /// numarul de stari, respectiv tranzitii
    std::vector<int> states;
    int initialState;
    std::unordered_set<int> finalStates;
    std::unordered_map<int, std::unordered_map<char,std::vector<int> > > tranz; /// tranzitii

    void runTests(std::istream& in,std::ostream& out)
    { /// fara accepted() implementat, functia asta nu are sens momentan.
        /// insa in clasele derivate, o sa o fac publica fara a o rescrie.
        int testCount;
        in >> testCount;
        for(int i=0;i<testCount;i++)
        {
            std::string word;
            in >> word;
            if(accepted(word))
            {
                out << "DA\n";
                std::cout << "Drumul este: ";
                path(word);
                std::cout << '\n';
            }
            else
            {
                out << "NU\n";
                std::cout << "Cuvantul nu e acceptat\n";
            }

        }
    }
public:
    FA():n(0){}
    void read(std::istream& in)
    {
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
            int startNode, endNode; /// starea de plecare si terminare a tranzitiei (muchiei)
            char letter; /// simbolul din limbaj.
            in >> startNode >> endNode >> letter;
            tranz[startNode][letter].push_back(endNode);
        }
        in >>initialState;
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
    virtual bool accepted(std::string word) = 0;
    virtual void path(std::string word) = 0;/// assumes word is accepted and displays path.
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
    using FA::runTests;
    bool accepted(std::string word){
        return accepted(word,false);
    }
    void path(std::string word){
        accepted(word,true);
    }
    bool accepted(std::string word,bool outputPath = false)
    {
        if(word.size() == 0)
            return (finalStates.find(initialState) != finalStates.end());
        int node = initialState;
        for(unsigned int i = 0; i < word.size(); i++)
        {
            if(tranz[node].find(word[i]) == tranz[node].end())
                return false;
            node = tranz[node][word[i]][0]; /// ar trebui sa existe doar o muchie pt litera curenta.
        }
        return (finalStates.find(node) != finalStates.end());
    }
};
class NFA : public FA
{
public:
    using FA::runTests;
    bool accepted(std::string word){return acceptedLONG(word,false);}
    void path(std::string word)    {
        acceptedLONG(word,true);
    }
    void printPath(std::deque<int> nodes, int currentNode, int lastNode)
    {
        while(!nodes.empty())
        {
            std:: cout << nodes.front() << ' ';
            nodes.pop_front();
        }
        std::cout << currentNode << ' ' << lastNode;
    }


    /// cel mai complicat DFS ever. N-am mai vazut pe net DFS care are
    /// stack de iteratori (ca sa aiba maxim O(n) memorie, nu O(n^2) pe un graf complet?
    bool acceptedLONG(std::string word,bool outputPath = false)
    {
        if(word.size() == 0)
            return (finalStates.find(initialState) != finalStates.end());
        if(tranz[initialState].find(word[0]) == tranz[initialState].end())
            return false;
        std::deque<int> prevNodes;
        prevNodes.push_back(initialState);
        std::stack< std::vector<int>::iterator > its;
        its.push(tranz[initialState][word[0]].begin());
        for(int i=1;i >= 0; i++)
        {
            if(its.empty())return false;
            if(its.top() == tranz[prevNodes.back()][word[i-1]].end())
            {
                i-=2;
                its.pop();
                prevNodes.pop_back();
                if(!its.empty())
                    its.top()++;
                continue;
            }
            int node = *(its.top());
            if(i == word.size()-1)
            {
                for(const auto& lastNode:tranz[node][word[i]])
                    if (finalStates.find(lastNode) != finalStates.end())
                    {
                        if(outputPath)
                            printPath(prevNodes,node,lastNode);
                        return true;
                    }
                i-=1;
                its.top()++;
                continue;
            }
            if(tranz[node].find(word[i]) == tranz[node].end())
            {
                i-=1;
                its.top()++;
                continue;
            }
            prevNodes.push_back(node);
            its.push(tranz[node][word[i]].begin());
        }
        return false;
    }
    /*
    bool acceptedDFS_recursiv(std::string word, int node,int i=0)
    {
        if(i == word.size())
            return (finalStates.find(node) != finalStates.end());

        if(tranz[node].find(word[i]) != tranz[node].end())
            for(const auto& nextNode:tranz[node][word[i]])
                if(acceptedDFS(word,nextNode,i+1))
                    return true;
        return false;
    }

    stack<int> pathDFS(std::string word, int node, int i=0)
    {
        if(i == word.size())
            if (finalStates.find(node) != finalStates.end())
            {
                stack<int> result =
            }

        if(tranz[node].find(word[i]) != tranz[node].end())
            for(const auto& nextNode:tranz[node][word[i]])
                if(acceptedDFS(word,nextNode,i+1))
                    return true;
        return false;
    }*/

    /// merge, dar e mai greu sa construiesti drumul.
    bool acceptedBFS(std::string word)
    {
        if(word.size() == 0)
            return (finalStates.find(initialState) != finalStates.end());
        std::unordered_set<int> nodes;
        nodes.insert(initialState);
        for(unsigned int i = 0; i < word.size(); i++)
        {
            std::unordered_set<int> oldNodes = nodes;
            nodes.clear();
            for(const auto& node:oldNodes)
            {
                if(tranz[node].find(word[i]) != tranz[node].end())
                    for(const auto& nextNode:tranz[node][word[i]])
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
class l_NFA : public FA
{
public:
    using FA::runTests;
    bool accepted(std::string word){return acceptedLONG(word,false);}
    void path(std::string word)    {
        acceptedLONG(word,true);
    }
    bool acceptEmpty()
    {
        if(finalStates.find(initialState) != finalStates.end())
            return true;
        for(const auto& lambdaNode:tranz[initialState][0])
            if(finalStates.find(lambdaNode) != finalStates.end())
                return true;
        return false;
    }
    bool acceptedLONG(std::string word, bool outputPath = false)
    {
        if(word.empty())
            return acceptEmpty();
        std::stack<std::pair<int,int> > stk;
        stk.push({initialState,0});

        while(!stk.empty())
        {
            std::unordered_set<int> same,viz;
            int node = stk.top().first;
            int i = stk.top().second;
            stk.pop();

            if(i == word.size())
            {
                if(finalStates.find(node) != finalStates.end())
                    return true;
                continue;
            }

            std::stack<int> startNodes;
            startNodes.push(node);
            same.insert(node);
            /// lambda tranzitions
            while(!startNodes.empty())
            {
                int sameNode = startNodes.top();
                startNodes.pop();
                for (const auto& nextNode:tranz[node]['0'])
                {
                    if(same.find(nextNode) == same.end())
                    {

                        same.insert(nextNode);
                        startNodes.push(nextNode);
                    }
                }
                /// normal tranzitions
                for (const auto& nextNode:tranz[sameNode][word[i]])
                    viz.insert(nextNode);
            }
            /// the unique nodes
            for (const auto& nextNode:viz)
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
