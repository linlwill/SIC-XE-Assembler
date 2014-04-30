#ifndef INSTRUCTIONS_INCLUDED
#define INSTRUCTIONS_INCLUDED
#include "primary.h"

class Instruction {
  private:
    bool valid;
  public:
    //std::string keyword;
    int format;
    int opcode;

    bool isValid(){
    	return valid;
    }//end validation check.  Screw ambiguity.

    Instruction(){
    	valid = false;
    }//end constructor

    Instruction(int format, std::string opcode){
      valid = true;
      Instruction::format = format;
      Instruction::opcode = ::forceInt(opcode);
    }//end good constructor
};

class UnrecognizedRegisterException{
  public:
    std::string errorCode;
    UnrecognizedRegisterException(std::string code = ""){
      errorCode = code;
    }
};//end exception

namespace reg {
	std::map<std::string,int> DB;
	bool DBisOn = false;
	void initDB(){
		if (reg::DBisOn) return;
		reg::DBisOn = true;
		DB["A"] = 0;
		DB["X"] = 1;
		DB["L"] = 2;
		DB["PC"] = 8;
		DB["SW"] = 9;
		DB["B"] = 3;
		DB["S"] = 4;
		DB["T"] = 5;
		DB["F"] = 6;
	}//end init

	int get(std::string keyword){
		initDB();
		int out = DB[keyword];
		if (!out && (keyword != "A")) throw UnrecognizedRegisterException(keyword);
		return out;
	}//end get
}//end namespace

namespace instructions {
	std::map<std::string,Instruction> DB;
	bool DBinitialized = false;
	void initDB(){
		if (DBinitialized) return;
		instructions::DBinitialized = true;
		DB["ADD"]=Instruction(3,"18");
		DB["ADDF"]=Instruction(3,"58");
		DB["ADDR"]=Instruction(2,"90");
		DB["AND"]=Instruction(3,"40");
		DB["CLEAR"]=Instruction(2,"b4");
		DB["COMP"]=Instruction(3,"28");
		DB["COMPF"]=Instruction(3,"88");
		DB["COMPR"]=Instruction(2,"a0");
		DB["DIV"]=Instruction(3,"24");
		DB["DIVF"]=Instruction(3,"64");
		DB["DVIR"]=Instruction(2,"9c");
		DB["FIX"]=Instruction(1,"c4");
		DB["FLOAT"]=Instruction(1,"c0");
		DB["HIO"]=Instruction(1,"f4");
		DB["J"]=Instruction(3,"3c");
		DB["JEQ"]=Instruction(3,"30");
		DB["JGT"]=Instruction(3,"34");
		DB["JLT"]=Instruction(3,"38");
		DB["JSUB"]=Instruction(3,"48");
		DB["LDA"]=Instruction(3,"00");
		DB["LDB"]=Instruction(3,"68");
		DB["LDCH"]=Instruction(3,"50");
		DB["LDF"]=Instruction(3,"70");
		DB["LDL"]=Instruction(3,"08");
		DB["LDS"]=Instruction(3,"6c");
		DB["LDT"]=Instruction(3,"74");
		DB["LDX"]=Instruction(3,"04");
		DB["LPS"]=Instruction(3,"d0");
		DB["MUL"]=Instruction(3,"20");
		DB["MULF"]=Instruction(3,"60");
		DB["MULR"]=Instruction(2,"98");
		DB["NORM"]=Instruction(1,"c8");
		DB["OR"]=Instruction(3,"44");
		DB["RD"]=Instruction(3,"d8");
		DB["RMO"]=Instruction(2,"ac");
		DB["RSUB"]=Instruction(3,"4c");
		DB["SHIFTL"]=Instruction(2,"a4");
		DB["SHIFTR"]=Instruction(2,"a8");
		DB["SIO"]=Instruction(1,"f0");
		DB["SSK"]=Instruction(3,"ec");
		DB["STA"]=Instruction(3,"0c");
		DB["STB"]=Instruction(3,"78");
		DB["STCH"]=Instruction(3,"54");
		DB["STF"]=Instruction(3,"80");
		DB["STI"]=Instruction(3,"d4");
		DB["STL"]=Instruction(3,"14");
		DB["STS"]=Instruction(3,"7c");
		DB["STSW"]=Instruction(3,"e8");
		DB["STT"]=Instruction(3,"84");
		DB["STX"]=Instruction(3,"10");
		DB["SUB"]=Instruction(3,"1c");
		DB["SUBF"]=Instruction(3,"5c");
		DB["SUBR"]=Instruction(2,"94");
		DB["SVC"]=Instruction(2,"b0");
		DB["TD"]=Instruction(3,"e0");
		DB["TIO"]=Instruction(1,"f8");
		DB["TIX"]=Instruction(3,"2c");
		DB["TIXR"]=Instruction(2,"b8");
		DB["WD"]=Instruction(3,"dc");
		//Begin memory management.  Format 0, opcode is length in bytes
		DB["WORD"] = Instruction(0,"3");
		DB["BYTE"] = Instruction(0,"1");
		DB["RESW"] = Instruction(0,"3");
		DB["RESB"] = Instruction(0,"1");
	}//end init

	Instruction get(std::string opor){
	    //Return the instruction indicated by opor.  Ignore any leading +.
		initDB();
		if (opor[0] == '+') opor.erase(0,1);
		return DB[opor];
	}//end get

	int sizeOf(std::string opor, std::string opand){
        //Retun the size, in bytes, of the instruction based on format and possibly operand
        Instruction theInst = get(opor);
        int size;

        if (theInst.format){
            //Instruction.  Format is the bytes it takes, +1 if opor begins with +
            size = theInst.format;
            if (opor[0] == '+') ++size;
        } else {
            //Memory.  opcode is length in bytes, multiply that by operand if reservation.
            size = theInst.opcode;
            if (opor.substr(0,3) == "RES")
                size *= ::forceInt(opand);
        }//end branch
        return size;
	}//end sizeOf - why has it taken me this long to write this function?
}//end namespace

#endif
