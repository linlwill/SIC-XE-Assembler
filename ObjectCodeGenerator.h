//Return the object code specified by a given line of assembly code.  Generate a modification record for extended-format instructions.
#ifndef OBJECTCODE_INCLUDED
#define OBJECTCODE_INCLUDED
#include "primary.h"
#include "Base.h"
#include "Modification.h"
#include "Instructions.h"

int toAddress(std::string token){
    //std::cout << "In toAddress working on " << token << std::endl;
    //Convert token into an address, compensating for all the syntax it could include.

    if (token[0] == '=')
        token.erase(0,1);

    int final;
    //Handle math with recursion
    Queue<std::string> mults = divideString(token,'*');
    if (mults.getLength() != 1){
        //Recurse to each term, then multiply them all together.
        final = toAddress(mults.pull());
        while (mults.notEmpty())
            final *= toAddress(mults.pull());
        return final;
    }//end multiplication
    //std::cout << "No multiplication" << std::endl;
    Queue<std::string> divs = divideString(token,'/');
    if (divs.getLength() != 1){
        final = toAddress(divs.pull());
        while (divs.notEmpty())
            final /= toAddress(divs.pull());
        return final;
    }//end division
    //std::cout << "No division" << std::endl;
    Queue<std::string> adds = divideString(token,'+');
    if (adds.getLength() != 1){
        final = toAddress(adds.pull());
        while (adds.notEmpty())
            final += toAddress(adds.pull());
        return final;
    }//end addition
    //std::cout << "No addition" << std::endl;
    Queue<std::string> subs = divideString(token,'-');
    if (subs.getLength() != 1){
        final = toAddress(subs.pull());
        while (subs.notEmpty())
            final -= toAddress(subs.pull());
        return final;
    }//end subtraction
    //std::cout << "No subtraction" << std::endl;

    //Only a primative will have reached this point. Figure out which labelTable we should be using and fetch from it.  * is a special case.
    if (token == "*") return ::currentAddress;

    std::map<std::string, int>& theTable = (::currentMacro) ? ::currentMacro->labels : ::labelTable;
    final = theTable[token];
    //std::cout << "Pulled " << final << " from the table." << std::endl;
    //If final is zero, we're either at the beginning or it's not a label.
    if (!final){
        //std::cout << "In the zero if" << std::endl;
        //If it's the zero-case, return zero plus the offset.
        std::string zeroCase;
        if (::currentMacro){
            zeroCase = ::currentMacro->zeroLabel;
        } else zeroCase = ::startLabel;

        if (token != zeroCase) {
            //Not a label at all.  Treat as a number then.
            //std::cout << "returning a number" << std::endl;
            return forceInt(token);
        }//end if-number
    }//end zero-case

    //std::cout << "Out of the zero-if" << std::endl;
    //Offset the address.  In a macro, by the start point.  In global, by how much space macros have taken up so far.
    int offset = (::currentMacro) ? ::cMacStartAddr : ::totalMacroOffset;
    return final + offset;
}//end addressing

std::string objectCode(std::string opor, std::string opand){
    Instruction theInst = instructions::get(opor);
    std::string finalCode = "";

    if (theInst.format == 0){
        //Memory management.  Opcode is length in bytes.  Opand is either the value to initialize to, or the number of things to reserve.
        int value = ::forceInt(opand);
        if (opor.substr(0,3) == "RES"){
            //Reservation.  Final code is a flag to end the text record.  Space is value*opcode
            finalCode = "!END!";
            ::currentAddress += theInst.opcode * value;
            if (::currentMacro) ::totalMacroOffset += theInst.opcode * value;
        } else {
            //Assignment.  Final code is hex equivilent of value.
            finalCode = hexOf(value,theInst.opcode);
            ::currentAddress += theInst.opcode;
            if (::currentMacro) ::totalMacroOffset += theInst.opcode;
        }//end branch
    }//end 0-case: memory

    else if (theInst.format == 1){
        //Simple one byte instruction.  Return opcode and advance current address by 1.
        finalCode = hexOf(theInst.opcode);
        ::currentAddress++;
        if (::currentMacro) ::totalMacroOffset++;
    }//end 1-case: easy

    else if (theInst.format == 2){
        //Two byte register action.  Break opand up by comma.  Object code is opcode + reg1 + reg2
        Queue<std::string> theRegs = divideString(opand,',');
        if (theRegs.getLength() > 2) throw Error("Invalid operand in type-2 instruction:\n"+opand);
        finalCode = hexOf(theInst.opcode);
        finalCode += reg::get(theRegs.pull());
        //Since some format 2 instructions take only a single operand, append a dummy-zero just to take up space.
        if (theRegs.notEmpty()) finalCode += reg::get(theRegs.pull());
        else finalCode += "0";

        //Advance current address by 2
        ::currentAddress += 2;
        if (::currentMacro) ::totalMacroOffset += 2;
    }//end 2-case: medium

    else if (theInst.format == 3){
        //Mode 3/4 instruction.  Here we go.  First: determine address.
        //bool literal = false;
        int address, modeFour = 0;
        std::string nixbpe = "000000";

        //Check for indexed:
        std::string withoutEnd = opand.substr(0,opand.length()-1);
        if (withoutEnd == ",X"){
            //Set x to 1 and strip the ,X
            nixbpe[2] = '1';
            opand = withoutEnd;
        }//end indexed

        //Check for prefixes.  Set coresponding bits.
        if (opand[0] == '@'){
            opand.erase(0,1);
            nixbpe[0] = '1';
        }//end indirect

        if (opand[0] == '='){
            //literal = true;
            opand.erase(0,1);
        }//end literal

        if (opand[0] == '#'){
            opand.erase(0,1);
            nixbpe[1] = '1';
        }//end immediate

        //Resolve the operand into an address (or a number; the differences are purely situational)
        address = toAddress(opand);

        //Check if mode-4
        if (opor[0] == '+'){
            opor.erase(0,1);
            nixbpe[5] = '1';
            modeFour = 1;
            //Length of a mode-4 is always 20 bytes.
            modRec::push(currentAddress,40);
        }//end mode-4

        //Resolve p and b.  Only run if not in mode i or e
        if ((nixbpe[5] == '0') && (nixbpe[1] == '0')){
            //Address is currently what we *want*.  We need to change it to what we *need*.
            int variance = address - currentAddress;
            if ((variance < 2047)&&(variance > -2048)){
                //We are in range for PC relative.
                nixbpe[4] = '1';
                address = variance;
            }//end PC-relative

            else if (Base::inBlock(::currentAddress)){
                //We are in a base block.  Let's see if we're close enough for that to matter.
                unsigned int bVariance = address - Base::getBase(currentAddress);
                if (bVariance < 4095){
                    nixbpe[3] = '1';
                    address = bVariance;
                }//end if
            }//end base relative

            //All hope is lost
            else throw Error("Out of range");
        }//end b/p

        //Flag bits have been determined.  Address has been resolved.  If n, add 2 to the opcode.  If i, add 1.  FinalCode is the hex of that, to two characters.
        int opcode = theInst.opcode;
        if (nixbpe[0] == '1')
            opcode += 2;
        if (nixbpe[1] == '1')
            opcode += 1;
        finalCode = hexOf(opcode,2);

        //Next four bits of finalCode are xbpe
        std::string xbpe = "B'";
        xbpe += nixbpe.substr(2,6);
        xbpe += "'";
        finalCode += hexOf(xbpe);

        //Next is the address, which takes up either 3 or 5 hex characters depending on e
        int len = (modeFour) ? 3 : 5;
        finalCode += hexOf(address,len);

        //Advance current address by 3 or 4
        ::currentAddress += (modeFour) ? 3 : 4;
        if (::currentMacro) ::totalMacroOffset += (modeFour) ? 3 : 4;
    }//end 3-case: hard

    return finalCode;
}//end objectCode

#endif
