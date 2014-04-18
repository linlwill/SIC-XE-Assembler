#ifndef BASE_INCLUDED
#define BASE_INCLUDED
#include "LinkedList.h"
//Provide functions to begin and end base blocks, then later check if where they are is in a block and if so, what that block's value is

class NotInBaseException{
  public:
    NotInBaseException(){}
};//end error

namespace Base {

class BaseBlock{
  public:
  int startAddress;
  int value;
  int endAddress;

  BaseBlock(){}

  BaseBlock(int a, int b, int c){
    startAddress = a;
    value = b;
    endAddress = c;
  }//end constructor
};//end class.  Would have been struct but class made LinkedList utilization easier.

bool openBaseBlock = false;
LinkedList<BaseBlock> BaseBlockList = LinkedList<BaseBlock>();

void startBlock(int address, int value);
void endBlock(int address);
bool inBlock(int address);
int getBase(int address);
int buffer[2];

void startBlock(int address, int value){
  //iff there isn't a block open already, open one with corresponding values.
  if (!openBaseBlock){
    openBaseBlock = true;
    buffer[0] = address;
    buffer[1] = value;
  }//end if
}//end startblock

void endBlock(int address){
  //iff there is an open block, finish the block and add it to the list
  if (openBaseBlock){
    openBaseBlock = false;
    BaseBlockList.add(BaseBlock(buffer[0],buffer[1],address));
  }//end if
}//end endblock

bool inBlock(int address){
  //O(N) search of the base blocks checking if address is in bounds
  for (int i = 0; i < BaseBlockList.getLength(); i++){
    //if ((address >= (*temp).startAddress)&&(address <= (*temp).endAddress)) return true;
    BaseBlock temp = BaseBlockList[i];
    if ((address >= temp.startAddress)&&(address <= temp.endAddress)) return true;
  }//end for
  return false;
}//end inblock

int getBase(int address){
  //Another search just like inblock.  This time return value.  Return -1 on not found, so this could be used as bool too.
  for (int i = 0; i < BaseBlockList.getLength(); i++){
    BaseBlock temp = BaseBlockList[i];
    if ((address >= temp.startAddress)&&(address <= temp.endAddress)) return temp.value;
  }//end for
  throw NotInBaseException();
}//end getBase

}//end namespace
#endif
