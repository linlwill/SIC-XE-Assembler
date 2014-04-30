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
        int currentAddress, argCount;
        std::string zeroLabel;
        std::string* arguments;

        Macro(){
            instructions = LinkedList<std::string*>();
            currentAddress = 0;
        }//end constructor
};//end class
#endif
