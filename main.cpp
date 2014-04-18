//Handle all manipulations of the file access
#include "primary.h"
#include <fstream>
#include "Directives.h"
#include "Instructions.h"
#include "ObjectCodeGenerator.h"
#include "Modification.h"
#include "Text.h"


int verify(std::string block){
    //Return 1,2,3 for instruction, directive, or macro.  0 for unrecognized.
    if (instructions::get(block).isValid())
        return 1;

    if (directives::get(block))
        return 2;

    /*macros goes here*/

    return 0;
}//end verify

int main(int argCount, char** args){
    //Only run if given an input
    //Initialize the shared data
    Queue<std::string*> instQueue = Queue<std::string*>();

    //Open the files
    std::ifstream inFile;
    if (argCount > 1) inFile.open(args[1]);
    else inFile.open("test.txt");

    //Error if the file didn't open
    if (!inFile.is_open()) throw Error("File did not open");

    std::ofstream outFile;
    if (argCount > 2) outFile.open(args[2]);
    else outFile.open("out.txt");

    //Pass One.  Iterate through the file, processing directives, adding labels, and pushing instructions to the queue.
    std::string line, workingBlock, label;
    int blockCount, state, temp;
    while(!inFile.eof()){
        getline(inFile,line);
        //Lines are partitioned by spaces and tabs.
        Queue<std::string> blocks = divideString(line, ' ');
        blockCount = blocks.getLength();

        workingBlock = blocks.pull();
        //If line numbers are included, meaning the first block begins with a number character, ignore that block.
        if (workingBlock[0] >= '0' && workingBlock[0] <= '9')
            workingBlock = blocks.pull();

        //First block is either an instruction, a directive, or unrecognized which means label.
        state = verify(workingBlock);
        label = "";

        if (state == 0){
            //Unrecognized.  Here in pass one, that means label.  Map workingBlock to the current address and grab the next block to continue
            ::labelTable[workingBlock] = ::currentAddress;
            if (::currentAddress == 0)
                ::startLabel = workingBlock;

            workingBlock = blocks.pull();
            label = workingBlock;
            state = verify(workingBlock);
        }//end label case

        if (state == 2){
            //Directive.  Build a string array of operands and pass to the directive processor.
            temp = directives::get(workingBlock);
            std::string* opands = new std::string[temp];

            //Directives asks for n arguments.  0 is label.  1-n are programmer-specified.  Queue will throw an error if insufficient.
            opands[0] = label;
            try {
                for(int i = 1; i < temp; i++)
                    opands[i] = blocks.pull();
            } catch(NotInQueueException){
                throw Error("Not enough arguments in line \n"+line);
            }//end exception handling

            directives::process(workingBlock, opands);
            delete[] opands;
        }//end directive-case

        else if (state == 1){
            //Instruction.  Assemble an instLine and push it to the instQueue, then update the current address.
            std::string I[2];
            I[0] = workingBlock;
            I[1] = blocks.pull();
            instQueue.push(I);

            //Update current address.
            Instruction theInst = instructions::get(workingBlock);

            if (theInst.format)
                ::currentAddress += (workingBlock[0] == '+') ? 4 : theInst.format;
            else {
                //Type-0, aka memory type.
                int size = theInst.opcode;

                if (workingBlock.substr(0,3) == "RES")
                    //Multiply the size by the amount of reservations taking place
                    size *= ::forceInt(I[1]);

                ::currentAddress += size;
            }//end else

        }//end instruction-case

        else if (state == 3){
            //Macro.  Not implemented yet.
        }//end macro-case

    }//End pass one.
    inFile.close();

    //Move on to pass two.  The instQueue is now filled with instLines.  First, create the header record.
    while(::programName.length() < 6) ::programName += ' ';
    outFile << 'H' << programName.substr(0,6) << ::hexOf(::startingAddress,6) << ::hexOf(::programLength,6) << std::endl;

    //Reset the current address and handle all the instructions.
    ::currentAddress = ::startingAddress;
    std::string* workingLine;
    while(instQueue.notEmpty()){
        workingLine = instQueue.pull();
        //Create the object code from the instruction and push it to the text records.
        textRec::push(objectCode(workingLine[0],workingLine[1]));
    }//end pass two

    //Pass the text records, the modification records, and then make/pass the end record
    textRec::push("!END!");
    while(textRec::notEmpty())
        outFile << textRec::pull() << '\n';
    while(modRec::notEmpty())
        outFile << modRec::pull() << '\n';

    //End record: E + starting address(6)
    outFile << 'E' << hexOf(::startingAddress,6);
    outFile.close();
}//end main
