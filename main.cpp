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

    if (::macroTable[block])
        return 4;

    return 0;
}//end verify


int main(int mainArgCount, char** mainArgs){
    /*********************
    Pass One

    Open the specified file (or testFile.txt).
    Read each line in.  If directive, assemble a set of operands (including label) and pass to the directive processor.
    If macro definition, map its label to a new macro and from now until MEND, push instructions to the macro's queue instead of the global queue.
    If instruction or macro invocation, assemble an opor/opand pair and push it to whatever queue we're using.  If instruction, update currentAddress.
    If label, eval the next block.  If instruction/invocation, map current address to the label.
    *********************/
    //Open the files.  Default to testFile.txt, else whatever the first passed thing is.
    std::ifstream inFile;
    if (mainArgCount > 1) inFile.open(mainArgs[1]);
    else inFile.open("testFile.txt");

    //Error if the file didn't open
    if (!inFile.is_open()) throw Error("File did not open");

    //Declare data we will need for line evaluation, and initialize the shared data.  Keep a global queue and a pointer to whatever queue we're working on now, which by default is global.  Macros will change this (and then change it back)
    ::currentMacro = 0;
    Queue<std::string*> globalInstQueue = Queue<std::string*>();
    Queue<std::string*>* theQueue = &globalInstQueue;

    std::string line, workingBlock, label, argLine, errorMessage, opor, opand;
    int state, argCount;
    std::string* stringBlock;

    //Begin reading lines in.
    while (!inFile.eof()){
        getline(inFile,line);
        //Empty lines are worthless.  Lines that begin with periods are comments.  In either case, move on like it wasn't even here.
        if ((line == "") || (line[0] == '.')) continue;

        //Reset the label field so we don't get stale data.
        label = "";

        //Whitespace (and tabs, but they're a special case within divideString) demark blocks.  Divide based on spaces.
        Queue<std::string> blocks = divideString(line,' ');
        workingBlock = blocks.pull();

        //If the first block's first character is a number, we're dealing with line numbers, and the first block is irrelevant.  Move on to the next.
        if ((workingBlock[0] >= '0') && (workingBlock[0] <= '9')){
            if (blocks.notEmpty()) workingBlock = blocks.pull();
            else throw Error("Line numbers given on an otherwise empty line");
        }//end line number handling

        //Branch behavior based on what the working block is.
        state = verify(workingBlock);
        if (state == 0){
            //Block was unrecognized, which means label (probably).  Store it, because it matters, and re-evaluate based on the NEXT block.
            label = workingBlock;
            if (blocks.notEmpty()) workingBlock = blocks.pull();
            else {
                //An unrecognized token with nothing afterward is an error.
                errorMessage = "Unrecognized token with no operand:\n";
                errorMessage += workingBlock;
                throw Error(errorMessage);
            }//end error creation.

            //Re-branch based on state of the new operand
            state = verify(workingBlock);

            if (state == 3){
                //The beginning of a macro.  If we're already in a macro, that's an error.
                if (::currentMacro){
                    errorMessage = "Macro definition was started when a macro was already being defined.  Label used for invalid macro was :";
                    errorMessage += label;
                    throw Error(errorMessage);
                }//end error generation
                else {
                    //Start a new macro.  Map it to the label.
                    Macro* newMacro = new Macro();
                    ::macroTable[label] = newMacro;
                    ::currentMacro = newMacro;
                    newMacro->argCount = 0;

                    //If any arguments were specified, build a list of them for the macro.  Else, specify that it's null.
                    if (blocks.notEmpty()){
                        //Divide based on comma.  Each entry is an argument.
                        argLine = blocks.pull();
                        Queue<std::string> argQueue = divideString(argLine,',');
                        //Build an array of strings for the macro, each string being an entry in the argument line.
                        argCount = argQueue.getLength();
                        newMacro->argCount = argCount;
                        newMacro->arguments = new std::string[argCount];

                        for(int i = 0; i < argCount; i++)
                            newMacro->arguments[i] = argQueue.pull();
                    //If no arguments were given, set the macro's arguments pointer to 0 as a flag that there are no arguments.  Have I mentioned that I love pointers?  They can be data, undefined, or flags that there's no data here.  References can't do that.
                    } else newMacro->arguments = 0;

                } //end yes/no branch based on ::currentMacro
            }//end macro start/end case

            else {
                //If this doesn't start/end a macro but it has a valid, it's a label.  If we're in a macro and the label begins with $, it's relative, map it relatively within the macro.  Else, map it globally.
                if ((::currentMacro) && (label[0] == '$'))
                    //Map the label to the macro's labelTable, using the relative address within the macro
                    ::currentMacro->labels[label] = ::currentMacro->currentAddress;
                else
                    //Map the label to the current location irrespective of any macros
                    ::labelTable[label] = ::currentAddress;
            }//end everything other than macro start/end.
        }//end 0, aka label-case

        //Don't do any else-ing right here, because labels will break off to here.
        if ((state == 1) || (state == 4)){
            //An instruction or a macro invocation.  In either case, assemble an opor/opand pair and add it to the list/queue
            std::string* pair = new std::string[2];
            opor = workingBlock;
            pair[0] = opor;
            if (blocks.notEmpty()) opand = blocks.pull();
            else opand = "";
            pair[1] = opand;

            //If in a macro, add it to its list.  Else, push it to the queue.
            if (::currentMacro)
                ::currentMacro->instructions.add(pair);
            else theQueue->push(pair);

            //For instructions only: update current address.  Either the global one or the macro's relative one.
            if (state == 1){
                int size = instructions::sizeOf(opor,opand);
                if (::currentMacro) ::currentMacro->currentAddress += size;
                else ::currentAddress += size;
            }//end instructions-only VIP room

            //The branch isn't necessary but it improves readability.  For an invocation, jump ahead the macro's size.
            else if (state == 4){
                int mSize = ::macroTable[opor]->size;
                ::currentAddress += mSize;
                if (::currentMacro) ::currentMacro->currentAddress += mSize;
            }//end invocations only

        }//end 1 or 4 aka instruction or invocation

        else if (state == 2){
            //Assemble a list of strings, each of them an operand (or a label).  Blanks are okay, sometimes that's acceptable.
            argCount = directives::get(workingBlock);
            stringBlock = new std::string[argCount];
            stringBlock[0] = label;
            if (blocks.notEmpty()) stringBlock[1] = blocks.pull();

            //Pass the set to the directives processor, then free the strings from memory
            directives::process(workingBlock, stringBlock);
            delete[] stringBlock;
        }//end 2, aka directive

        else if (state == 3){
            //Macro beginning/end.  We handled beginning within the label field, so here we will handle ending.
            if (workingBlock == "MEND"){
                //Set the macro's size to its maximal address, reset its current address to 0, then the current macro to 0 as a flag that we are done.
                ::currentMacro->size = ::currentMacro->currentAddress;
                ::currentMacro->currentAddress = 0;
                ::currentMacro = 0;
            }//end restriction to ending
        }//end 3, aka macro end

    }//end pass-one while.  Close the file, we're done with it.
    inFile.close();


    //Throw an error if we exited pass 1 while leaving an open macro
    if (::currentMacro) throw Error("Pass One over with open macro definition");

    /*********************
    Pass Two

    Reset the current location to whatever was given as the START address.
    Pull items off the queue.  If instruction, generate object code and push to the text records.
    If invocation, iterate through the macro's list.  If any arguments were specified, map them to the given value, using toAddress from objectCode.
    *********************/

    //Reset/declare data for pass 2
    ::currentAddress = startingAddress;
    std::string* workingLine;
    int size;

    while(globalInstQueue.notEmpty()){
        //Pull an instruction.  First is operator, second is operand.
        workingLine = globalInstQueue.pull();
        opor = workingLine[0];
        opand = workingLine[1];

        //Check to see if it is a macro invocation.
        if (macroTable[opor]){
            //Enter the macro, set it's relative address to 0.
            ::currentMacro = macroTable[opor];
            LinkedList<std::string*>& theList = currentMacro->instructions;
            currentMacro->currentAddress = 0;
            ::cMacStartAddr = ::currentAddress;

            //If there are arguments to be had, opand needs to be broken up and assigned accordingly.  Passing false to dS allows empty arguments.
            if (::currentMacro->arguments){
                Queue<std::string> givenArgs = divideString(opand,',',false);
                std::string theArg;
                int theAssignment;
                for (int i = 0; i < ::currentMacro->argCount; i++){
                    //If there is an argument given, assign it.  Else, if it includes an equals, let it be that.  Else, error.
                    theArg = ::currentMacro->arguments[i];
                    if (givenArgs.notEmpty()) theAssignment = toAddress(givenArgs.pull());
                    else if (macros::hasDefault(theArg)) theAssignment = toAddress(macros::getDefault(theArg));
                    else throw Error("Insufficient arguments given in macro invocation");

                    ::currentMacro->labels[theArg] = theAssignment;
                }//end for
            }//end if

            //Iterate through its instructions, generating code for them, adding them to the text records.
            for(int i = 0; i < theList.getLength(); i++){
                //For now, assume no macros-in-macros
                workingLine = theList[i];
                opor = workingLine[0];
                opand = workingLine[1];

                textRec::push(objectCode(opor,opand));

                //Update the current location and relative location.
                size = instructions::sizeOf(opor,opand);
                ::currentAddress += size;
                ::currentMacro->currentAddress += size;
            }//end for

        }//end macro invocation

        else {
              //Not a macro.  Assemble some object code and push it to the text records.
              textRec::push(objectCode(opor,opand));
              //Update the current location.
              ::currentAddress += instructions::sizeOf(opor,opand);
        }//end instruction
    }//end pass-2 while


    /*********************
    Code Generation

    Open the outFile for writing.
    Write the header record, all the text records, the modification records, and the end record, in that order.
    *********************/
    //Mandate an out.txt.  This is to allow forwards-compatibility with any multi-file nonsense later.
    std::ofstream outFile;
    outFile.open("out.txt");

    outFile << 'H' << programName.substr(0,6) << ::hexOf(::startingAddress,6) << ::hexOf(::programLength,6) << std::endl;

    //Push a final end flag to ensure no data gets left behind.
    textRec::push("!END!");
    while(textRec::notEmpty())
        outFile << textRec::pull() << '\n';
    while(modRec::notEmpty())
        outFile << modRec::pull() << '\n';

    //End record: E + starting address(6)
    outFile << 'E' << hexOf(::startingAddress,6);
    outFile.close();
}//end main
