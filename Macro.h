#ifndef MACROS_INCLUDED
#define MACROS_INCLUDED
#include <map>
#include <string>
#include "LinkedList.h"

class Macro {
    //Instructions are held in len-2 arrays of strings

    public:
        //Queue<std::string*> instructions;
        LinkedList<std::string*> instructions;
        std::map<std::string,int> labels;
        int currentAddress, argCount, size;
        std::string zeroLabel;
        std::string* arguments;

        Macro(){
            instructions = LinkedList<std::string*>();
            currentAddress = 0;
        }//end constructor
};//end class

namespace macros {
    bool hasDefault(std::string opand){
        //Return true if the string contains an equals
        for (int i = 0; i < opand.length(); i++)
            if (opand[i] == '=') return true;
        return false;
    }//end has default

    std::string getDefault(std::string opand){
        //Return everything after the first =.
        std::string buffer = "";
        bool go = false;
        for (int i = 0; i < opand.length(); i++){
            if (go) buffer += opand[i];
            else go = (buffer[i] == '=');
        }//end for;
        if (go) return buffer;
        else return "!!!ERROR!!!";
    }//end getDefault

}//end namespace
#endif
