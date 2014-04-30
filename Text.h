#ifndef TEXTRECORD_INCLUDED
#define TEXTRECORD_INCLUDED
#include "primary.h"
namespace textRec {
    Queue<std::string> records = Queue<std::string>();
    std::string openRecord = "";
    int startAddr;

    void push(std::string code){
       if (code == "!END!"){
            //A reservation, or other call to end the text record, has been encountered.  Package it all up and push to the queue.
            if (openRecord == "") return;
            int finalLength = ::currentAddress - startAddr;
            std::string finalCode;
            //T + starting address(6) + length in bytes(2) + text(60).  2-byte length means max bytes is 255, which is being rounded down to 240, aka f0 or 60 hex characters.
            while(finalLength > 240){
                finalLength -= 240;
                finalCode = "T" + ::hexOf(startAddr,6) + "^f0^" + openRecord.substr(0,60) + "^";
                startAddr += 240;
                records.push(finalCode);
            }//end while
            finalCode = "T" + ::hexOf(startAddr,6) + "^" + ::hexOf(finalLength,2) + "^" + openRecord;
            records.push(finalCode);
            openRecord = "";
        }//end if
        else {
            //If no open record, start one.  It's starting address is the current address (b/c this record will be written before current address is updated)
            //code is a hex string, each hex string is half a byte.
            if (openRecord == "") startAddr = ::currentAddress;
            openRecord += code;
        }//end else
    }//end push

    std::string pull(){
        return records.pull();
    }//end string

    bool notEmpty(){
        return records.notEmpty();
    }//end bool

}//end namespace
#endif
