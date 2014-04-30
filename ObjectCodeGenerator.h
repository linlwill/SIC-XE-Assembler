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
    /****************
    Object Code Generation
    Fetch the coresponding instruction from opor.  Based on its format, evaluate the necessary object code using opand.
    ****************/
    
    Instruction theInst = instructions::get(opor);
    std::string finalCode = "";
    
    if (theInst.format == 0){
        //Memory management.  Branch based on whether this is assignment or reservation
        if (opor.substr(0,3) == "RES"){
            //Reservation.  End the current text record.  Movement ahead in space will occur outside this function.
            finalCode = "!END!";
        }//end reservation
        else {
            //Assignment.  Opand is the value.  Return the hexidecimal equivilent of that.  Opcode holds the byte count, each byte is 2 hex characters.
            int value = addressOf(opand);
            finalCode = hexOf(value,theInst.opcode*2);
        }//end assignment
    }//end memory
    
    else if (theInst.format == 1){
        //Simple, one byte opcode.  Return hex of opcode in one byte
        finalCode = hexOf(theInst.opcode,2);
    }//end mode-1
    
    else if (theInst.format == 2){
        //Two bytes: opcode, R1, and possibly R2.  Rs are divided in operand by a comma.  If R2 is absent, fill in f as a placeholder since f is an invalid register.
        finalCode = hexOf(theInst.opcode,2);
        Queue<std::string> theRegs = divideString(opand,',');
        if (theRegs.notEmpty()){
            int R1 = regs::get(theRegs.pull());
            int R2 = 15;
            if (theRegs.notEmpty()) R2 = regs::get(theRegs.pull());
        } else throw Error("No operand given in mode-2 instruction");
        //Convert the numerical representation of the registers to hex and append them to the code
        finalCode += hexOf(R1,1);
        finalCode += hexOf(R2,1);
    }//end mode-2
    
    else if (theInst.format == 3){
        //Three, possibly four bytes.  Flag bits based on syntax and nature of operand.
        int xbpe = 0;
        int ni = 0;
        bool modeFour = false;
        bool immediate = false;
        
        //E is determiend by leading + in opor
        if (opor[0] == '+'){
            modeFour = true;
            opor.erase(0,1);
            xbpe += 1;
        }//end extended-format handling
        
        //N and I depend on leanding char of operand
        if (opand[0] == '#'){
            ni += 1;
            opand.erase(0,1);
            immediate = true;
        }//end immediate
        if (opand[0] == '@'){
            ni += 2;
            opand.erase(0,1);
        }//end indirect
        
        //N and I are encoded within the opcode.  Add ni to the opcode to find the merged, 8-bit, outcome.
        int iOpcode = theInst.opcode + ni;
        std::string hOpcode = hexOf(iOpcode,2);
        
        //X is determined by last two chars of opand
        std::string lastTwo = opand.substr(opand.length()-1,opand.length());
        if (lastTwo == ",X"){
            xbpe += 8;
            opand = opand.substr(0,opand.length()-1);
        }//end indexed
        
        //B and P are determined by final address and the proximity thereof to currentAddress.  They default to 0 in mode 4 and immediate.
        int address = addressOf(opand);
        if ((!modeFour)&&(!immediate)){
            //To use p, program-counter relative addressing, we must be +- 2048 from the current address.
            int pVariance = address - ::currentAddress;
            if ((pVariance < 2047) && (pVariance > -2048)){
                xbpe += 2;
                address = pVariance;
            //End attempt for p
            } else if (Base::inBlock(::currentAddress)){
                //Out of range for p, but if we are in a base block then b may be possible.
                unsigned int bVariance = address - Base::getBase(currentAddress);
                if (bVariance < 4095){
                    xbpe += 4;
                    address = bVariance;
                //End attempt for b
                } else throw Error("Out of range for P and B")
            } else throw Error("Out of range for P and not in a base block");
        }//end BP
        
        //All flag bits have been determined.  Together they form a single hex character
        std::string hFlags = hexOf(xbpe,1);
        
        //Last is the determination of address, either 3 or 5 hex characters.
        int addrLen = (modeFour) ? 5 : 3;
        std::string hAddress = hexOf(address,addrLen);
        
        //Final code is opcode (with absorbed NI), XBPE, address
        finalCode = hOpcode + hFlags + hAddress;
    }//end mode-3
    
    return finalCode;
}//end object code

#endif
