#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <algorithm>

class DFA
{
    public:
    int n,m; /// numarul de stari, respectiv tranzitii
    std::vector<int> states;
    int initialState;
    std::map<int,bool> finalStates;
    std::map<std::pair<int,char>,int> tranz; /// tranzitii
    public:
    DFA():n(0){}
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
            tranz[std::pair<int,char>(startNode,letter)] = endNode;
        }
        in >>initialState;
        std::string listFinals;
        int nrFinalStates;
        in >> nrFinalStates;
        for(int i=0;i<nrFinalStates;i++)
        {
            int state;
            in >> state;
            finalStates[state] = true;
        }
    }
    bool accepted(std::string word)
    {
        int node = initialState;
        for(int i = 0; i < word.size(); i++)
        {
            std:: pair<int,char> t(node,word[i]);
            if(tranz.find(t) == tranz.end())
                return false;
            node = tranz[t];
        }
        return (finalStates.find(node) != finalStates.end());
    }
    void runTests(std::ifstream& in,std::ofstream& out)
    {
        int testCount;
        in >> testCount;
        for(int i=0;i<testCount;i++)
        {
            std::string word;
            in >> word;
            if(accepted(word))
                out << "DA\n";
            else
                out << "NU\n";
        }
    }
    friend std::istream& operator>>(std::istream& in, DFA& myDFA);
};
std::istream& operator>>(std::istream& in, DFA& myDFA)
{
    myDFA.read(in);
    return in;
}

int main()
{
    std::ifstream fin("input.txt");
    std::ofstream fout("output.txt");
    DFA automata;
    fin >> automata;
    automata.runTests(fin,fout);
}
