#ifndef MAIN_H_INCLUDED
#define MAIN_H_INCLUDED

#include <iostream>

using namespace std;

void readFree(Device&, ostream&, daddr_x);
void readInodes(Device&, ostream&, ino_x);
void readInodePermissions(dinode*, ostream&);
void readLevelBlocks(Device&, daddr_x*, ostream&);
void readLevel2Blocks(Device&, daddr_x*, ostream&);
void readLevel3Blocks(Device&, daddr_x*, ostream&);

#endif // MAIN_H_INCLUDED
