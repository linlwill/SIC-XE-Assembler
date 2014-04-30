#ifndef PRIMARY_INCLUDED
#define PRIMARY_INCLUDED
//Hold data that unconnected files use.  Import data-structure headers that operate independantly
#include "LinkedList.h"
#include "Queue.h"
#include <string>
#include <math.h>
#include <iostream>
#include <map>
#include "DivideString.h"
#include "Macro.h"

///Begin shared data structures that don't get their own header
class Error {
    //Generic error that prints text to the console
    public:
        Error(std::string text = ""){
            std::cout << "ERROR: " << text << std::endl;
        }//end constructor
};//end error

///Begin shared data
std::map<std::string, int> labelTable;
int currentAddress, startingAddress, programLength;
std::string programName, startLabel;
Macro* currentMacro;
int cMacStartAddr;
std::map<std::string, Macro*> macroTable;
//std::map<std::string, Queue<std::string*>*> progBlocks;

///Begin shared functions

int forceInt(std::string input){
    int base = 10;
    std::string working;
    bool reduce = true;
    //Check for alternate bases only if the string is long enough.
    if (input.length() > 3){
        //Return the integer representation of the input string, allowing X'working', C'working', and B'working' to branch mathematical base.
        std::string id = input.substr(0,2);
        working = input.substr(2,input.length()-3);
        if (id == "X'")
            base = 16;
        else if (id == "B'")
            base = 2;
        else if (id == "C'"){
            //Working is a string of chars that the programmer wants to preserve exactly as-is.  Chars in ASCII range are 8-bit, so treat as base-256
            base = 256;
            reduce = false;
        }//end char-case
        else
            //Integer case means no id/working; entire input string is numbers.
            working = input;
    } else working = input;
    //Integer representation is the sum of each character multiplied by its signifiance multiplied by its base.
    //'a' = 97, '0' = 48.  b > a, 1 > 0.
    int c, significance = 0, sum = 0;

    for(int i = working.length()-1; i >= 0; --i){
        //Work on the i-th character, moving downwards so first char worked on is least mathematically significant
        c = working[i];
        //Reduce integer value of the character by its ASCII offset, unless we're in char mode
        if (reduce){
            if (c >= 'a') c -= 87;
            else c -= '0';
        }//end reduction

        //Multiply true value of char by its base to the power of its significance, which is incremented for next time.  Add it to the sum
        sum += c * pow(base,significance++);
    }//end for
    return sum;
}//end string to int

const std::string hexDigits = "0123456789abcdef";
std::string hexOf(int value, int length = 0){
    //Return the hexidecimal representation of the value.  If length is given, add leading 0s until it is of that length.  Error if requested length is less than hex value.
    std::string final = "";

    if (!value){
        //Zero is a special case.  Return length zeros.
        while(length--) final += '0';
    }//end zero case
    else {
        //Use the quotient/remainder method of converting bases as described in 340.
        unsigned int quo = value;
        int rem;
        while (quo){
            rem = quo % 16;
            quo = quo / 16;
            final = hexDigits[rem] + final;
        }//end while

        //If a length was given, append zeros or throw an error.  If not, return as-is.
        if (length){
            if (length < final.length())
                final = final.substr(0,length);
            while (length > final.length())
                final = '0' + final;
        }//end length-forcing

    }//end nonzero case
    return final;
}//end int to hex-string

std::string hexOf(std::string input, int length = 0){
    return hexOf(forceInt(input),length);
}//end sugary merging of forceInt and hexOf

#endif
