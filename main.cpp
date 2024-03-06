#include <iostream>
#include <fstream>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <algorithm>
#include <stack>

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
    bool accepted(std::string word)
    {
        int node = initialState;
        for(unsigned int i = 0; i < word.size(); i++)
        {
            if(tranz[node].find(word[i]) == tranz[node].end())
                return false;
            node = tranz[node][word[i]][0]; /// ar trebui sa existe doar o muchie pt litera curenta.
        }
        return (finalStates.find(node) != finalStates.end());
    }
    void path(std::string word)
    {
        int node = initialState;
        std::cout << node;
        for(unsigned int i = 0; i < word.size(); i++)
        {
            node = tranz[node][word[i]][0];
            std::cout << "->" << node;
        }
    }
};
class NFA : public FA
{
public:
    using FA::runTests;

    /// cel mai complicat DFS ever. N-am mai vazut pe net DFS care are
    /// stack de iteratori (ca sa aiba maxim O(n) memorie, nu O(n^2) pe un graf complet?
    bool accepted(std::string word){
        if(word.size() == 0)
            return (finalStates.find(initialState) != finalStates.end());
        if(tranz[initialState].find(word[0]) == tranz[initialState].end())
            return false;
        std::stack<int> prevNodes;
        prevNodes.push(initialState);
        std::stack< std::vector<int>::iterator > its;
        its.push(tranz[initialState][word[0]].begin());
        for(int i=1;i >= 0; i++)
        {
            if(its.empty())return false;
            if(its.top() == tranz[prevNodes.top()][word[i-1]].end())
            {
                i-=2;
                its.pop();
                prevNodes.pop();
                if(!its.empty())
                    its.top()++;
                continue;
            }
            int node = *(its.top());

            if(i == word.size()-1)
            {
                for(const auto& lastNode:tranz[node][word[i]])
                    if (finalStates.find(lastNode) != finalStates.end())
                        return true;
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
            prevNodes.push(node);
            its.push(tranz[node][word[i]].begin());
        }
        return false;
    }
    void path(std::string word)    {

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


int main()
{
    std::ifstream fin("input.txt");
    std::ofstream fout("output.txt");
    NFA automata;
    fin >> automata;
    automata.runTests(fin,fout);
}
