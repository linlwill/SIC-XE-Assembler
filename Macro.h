#ifndef MACROS_INCLUDED
#define MACROS_INCLUDED
//Define the Macro class for use elsewhere.
#include <map>
#include <string>

class Macro {
    //Instructions are held in len-2 arrays of strings

    public:
        Queue<std::string*> instructions;
        std::map<std::string,int> labels;
        int currentAddress;
        std::string zeroLabel;

        Macro(){
            instructions = Queue<std::string*>();
            currentAddress = 0;
        }//end constructor

        std::string* nextI(){
            return instructions.pull();
        }//end next

        bool notEmpty(){
            return instructions.notEmpty();
        }//end not-empty
};//end class
#endif
