#ifndef SLICEASTRING_INCLUDED
#define SLICEASTRING_INCLUDED
#include <string>
#include "Queue.h"
//Divide a string up by a demarkation char. Return a queue full of those blocks.

Queue<std::string> divideString(std::string input, char demark, bool continuous = true){
    Queue<std::string> list = Queue<std::string>();
    std::string buffer = "";
    for (int i = 0; i < input.length(); i++){
        //9 is the char value of a tab.  Tabs always demark.
        if ((input[i] == demark) || (input[i] == 9)){
            //Doing continuous, strings of demarks count as one.  Turn cont off, and Hello!!World demarked with '!' will make a list {"Hello","","World"}
            if (buffer.length() || !continuous){
                list.push(buffer);
                buffer = "";
            }//end if
        }//end if-demark
        else {
            buffer += input[i];
        }//end else - aka not demark
    }//end for
    list.push(buffer);
    return list;
}//end divide

#endif
