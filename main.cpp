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

    if ((block == "MACRO") || (block == "MEND"))
        return 3;

    return 0;
}//end verify

int main(int argCount, char** args){
    //Only run if given an input
    //Initialize the shared data
    Queue<std::string*> instQueue = Queue<std::string*>();
    ::currentMacro = 0;

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
            //Unrecognized.  Here in pass one, that means label.  Next block is operator.
            label = workingBlock;
            workingBlock = blocks.pull();
            state = verify(workingBlock);

            //If macro, add to macroTable.  If at address 0, labelTable would lie so set the exception.  Else, add the label.
            if(state == 3)
                ::macroTable[label] =  new Macro();
            else{
                //Map the label to the address.  Global unless in a macro and beginning with $
                if (::currentMacro && (label[0] == '$'))
                    ::currentMacro->labels[label.erase(0,1)];
                else if (::currentAddress)
                    ::labelTable[label] = ::currentAddress;
                else //If currentAddress is 0, mapping it is a waste of time since 0 is the false-state.  Set the special case instead.
                    ::startingAddress = ::currentAddress;
            }//end else
        }//end label case

        if (state == 2){
            //Directive.  Build a string array of operands and pass to the directive processor.  No need for special macro-level behavior since no directives should show up in the macro definition.
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
            //Instruction.  Assemble an instLine and push it to the queue, then update the current address.  Whether this is global or macro-level depends on currentMacro.
            Queue<std::string*>& theQueue = (::currentMacro) ? instQueue : ::currentMacro->instructions;
            int& address = (::currentMacro) ? ::currentAddress : ::currentMacro->currentAddress;

            std::string I[2];
            I[0] = workingBlock;
            I[1] = blocks.pull();
            theQueue.push(I);

            //Update current address.
            Instruction theInst = instructions::get(workingBlock);

            if (theInst.format)
                address += (workingBlock[0] == '+') ? 4 : theInst.format;
            else {
                //Type-0, aka memory type.
                int size = theInst.opcode;

                if (workingBlock.substr(0,3) == "RES")
                    //Multiply the size by the amount of reservations taking place
                    size *= ::forceInt(I[1]);

                address += size;
            }//end else

        }//end instruction-case

        else if (state == 3){
            //We are entering or leaving a macro.  If leaving, set to 0.  If entering, set to address of the macro we just made.
            ::currentMacro = (::currentMacro) ? 0 : ::macroTable[label];
        }//end macro-case

    }//End pass one.
    inFile.close();

    //Move on to pass two.  The instQueue is now filled with instLines.  First, create the header record.
    while(::programName.length() < 6) ::programName += ' ';
    outFile << 'H' << programName.substr(0,6) << ::hexOf(::startingAddress,6) << ::hexOf(::programLength,6) << std::endl;

    //Reset the current address and handle all the instructions.
    ::currentAddress = ::startingAddress;
    ::totalMacroOffset = 0;
    std::string opor;
    std::string* workingLine;
    int vars;

    while(instQueue.notEmpty()){
        workingLine = instQueue.pull();
        opor = workingLine[0];

        //Did we just pull a macro?  macroTable maps to pointers, and 0 on false.  I love pointers.
        if (::macroTable[opor]){
            //Microcosm this whole process for the macro we just pulled.  Start from relative-zero.
            ::currentMacro = ::macroTable[opor];
            ::currentMacro->currentAddress = 0;
            ::cMacStartAddr = ::currentAddress;
            while(::currentMacro->notEmpty()){
                //Grab an instruction.  Is it a macro-directive?  If not, handle it just like we would outside a macro.  If so, outsource to the macro directive processor.
                workingLine = ::currentMacro->nextI();
                opor = workingLine[0];
                /* The beginnings of macro-directive handling is here.  I've commented it out because I wanted to upload the basic version first.
                vars = macroDirs::get(opor);

                if (vars){
                    //Assemble a dynamic array of strings corresponding to how many variables specified.  0 is opor.  Comma seperates arguments.
                    std::string* arguments = new std::string[vars];
                    arguments[0] = opor;
                    LinkedList<std::string> argList = divideString(workingLine[1], ',');
                    if (argList.getLength() >= vars) throw Error("Insufficent arguments given in macro invocation");


                }//End macro-directive-processing
                else */
                    textRec::push(objectCode(opor,workingLine[1]));
            }//end while macro not empty - leave the macro and resume pulling from the main queue
            ::currentMacro = 0;
        } //End macro-handling
        else
            //Create the object code from the instruction and push it to the text records.
            textRec::push(objectCode(opor,workingLine[1]));
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
