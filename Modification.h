#ifndef MODIFICATION_INCLUDED
#define MODIFICATION_INCLUDED
#include "primary.h"
namespace modRec {
    //Modification records

    Queue<std::string> records = Queue<std::string>();

    bool notEmpty(){
        return records.notEmpty();
    }//end notEmpty

    void push(int target, int length){
        //generate a new modification record, with target in len-6 hex and length in len-2
        std::string temp = "M";
        temp += ::hexOf(target,6);
        temp += ::hexOf(length,2);

        records.push(temp);
    }//end add

    std::string pull(){
        //Pull the leading modification record and return it
        return records.pull();
    }//end sugar

}//end namespace
#endif
