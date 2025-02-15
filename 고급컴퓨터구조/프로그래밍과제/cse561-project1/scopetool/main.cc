#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "printline.h"
#include <queue>
#include <string>
#include <iostream>
#include <map>

#define DIR_LENGTH	512

#define NUM_ARCH_REGS 67
#define NUM_PHYS_REGS (NUM_ARCH_REGS * 2)
struct MaptableEntry {
    int physReg;  
    bool ready;   
};

std::map<int, MaptableEntry> maptable;

std::queue<int> freelist; 

struct Instruction {
	unsigned int PC;
	int operationType;
	int destReg;
	int src1Reg;
	int src2Reg;
	
	Instruction(unsigned int pc, int opType, int dest, int src1, int src2)
        : PC(pc), operationType(opType), destReg(dest), src1Reg(src1), src2Reg(src2) {}
        
        Instruction parseInstruction(const std::string& traceLine) {
    		unsigned int pc;
    		int opType, dest, src1, src2;

    		std::sscanf(traceLine.c_str(), "%x %d %d %d %d", &pc, &opType, &dest, &src1, &src2);

    		return Instruction(pc, opType, dest, src1, src2);
	}	
};

class PipelineRegister {
public:
    	int maxSize;
    	std::queue<Instruction> instructions;

    	PipelineRegister(int size) : maxSize(size) {}

    	bool isEmpty() {
        	return instructions.empty();
    	}

    	void push(const Instruction& inst) {
        	if (instructions.size() < maxSize) {
            		instructions.push(inst);
        	} else {
            		std::cerr << "Pipeline register is full!" << std::endl;
        	}
    	}

    	Instruction pop() {
        	if (!isEmpty()) {
            		Instruction inst = instructions.front();
            		instructions.pop();
            		return inst;
        	} else {
            		throw std::out_of_range("Pipeline register is empty!");
        	}
    	}

    	int size() {
        	return instructions.size();
    	}
};

PipelineRegister DE(WIDTH); // Fetch -> Decode
PipelineRegister RN(WIDTH); // Decode -> Rename
PipelineRegister DI(WIDTH); // Rename -> Dispatch
PipelineRegister RR(WIDTH); // Issue -> Register Read
PipelineRegister execute_list(WIDTH * 5); // Register Read -> Execute
PipelineRegister WB(WIDTH * 5); // Execute -> Writeback

class InstructionQueue {
public:
    	int maxSize;
    	int freeEntries = maxSize;
    	std::queue<Instruction> instructions;

    	InstructionQueue(int size) : maxSize(size), freeEntries(size) {}

    	void enqueue(const Instruction& inst, bool src1Ready, bool src2Ready, int Bday) {
        	if (instructions.size() < maxSize) {
            		if (freeEntries > 0) {
            			instructions.push(inst);
            			--freeEntries;
            		}
        	} else {
            		std::cerr << "Instruction Queue is full!" << std::endl;
        	}
    	}

    	Instruction dequeue() {
        	if (!instructions.empty()) {
            		Instruction inst = instructions.front();
            		instructions.pop();
            		++freeEntries;
            		return inst;
        	} else {
            		throw std::out_of_range("Instruction Queue is empty!");
        	}
    	}

    	bool isEmpty() {
        	return instructions.empty();
    	}
};

class ReorderBuffer {
public:
    	int maxSize;
    	std::queue<Instruction> instructions;

    	ReorderBuffer(int size) : maxSize(size) {}

    	void enqueue(const Instruction& inst) {
        	if (instructions.size() < maxSize) {
            		instructions.push(inst);
        	} else {
            		std::cerr << "Reorder Buffer is full!" << std::endl;
        	}
    	}

    	Instruction dequeue() {
        	if (!instructions.empty()) {
            		Instruction inst = instructions.front();
            		instructions.pop();
            		return inst;
        	} else {
            		throw std::out_of_range("Reorder Buffer is empty!");
        	}
    	}

    	bool isEmpty() {
        	return instructions.empty();
    	}
    	
    	bool ready;
};

InstructionQueue IQ(IQ_SIZE);
ReorderBuffer ROB(ROB_SIZE);

void create_html(char *out) {
	char dir[DIR_LENGTH];
	char name[256];
	FILE *fp_temp;
	FILE *fp_html;

	sprintf(name, "%s.html", out);
	fp_temp = fopen(name, "r");
	if (fp_temp) {
	   fprintf(stderr, "HTML file `%s' already exists, exiting...\n", name);
	   exit(-1);
	}

	fp_html = fopen(name, "w");
	if (!fp_html) {
	   fprintf(stderr, "Cannot create HTML file `%s', exiting...\n", name);
	   exit(-1);
	}

	fprintf(fp_html, "<html>\n\n");

	if (!getcwd(dir, DIR_LENGTH)) {
	   fprintf(stderr, "Error while creating HTML file: cannot get current directory pathname, exiting...\n");
	   exit(-1);
	}

	fprintf(fp_html, "Click <a href=\"%s/%s\">HERE</a> for scope.\n\n",
				dir, out);

	fprintf(fp_html, "</html>\n");
	fclose(fp_html);
}

bool advance_cycle() {
 // advance_cycle() performs several functions.
 // (1) It advances the simulator cycle.
 // (2) When it becomes known that the pipeline is empty AND the
 // trace is depleted, the function returns “false” to terminate
 // the loop.
}

void fetch(FILE* fp_in) {
 // Do nothing if
 // (1) there are no more instructions in the trace file or
 // (2) DE is not empty (cannot accept a new decode bundle)
 //
	if (!DE.isEmpty || feof(fp_in)) {
        	return;
    	}
 // If there are more instructions in the trace file and if DE
 // is empty (can accept a new decode bundle), then fetch up to
 // WIDTH instructions from the trace file into DE. Fewer than
 // WIDTH instructions will be fetched and allocated in the ROB
 // only if
 // (1) the trace file has fewer than WIDTH instructions left.
 // (2) the ROB has fewer spaces than WIDTH.
 	int numInstructionsToFetch = WIDTH;
 		
    	int availableInstructionsInTrace = 0;
    	long currentPos = ftell(fp_in); 
    	fseek(fp_in, 0, SEEK_END); 
    	long fileSize = ftell(fp_in); 
    	fseek(fp_in, currentPos, SEEK_SET); 

    	while (availableInstructionsInTrace < WIDTH && fgets(line, sizeof(line), fp_in)) {
        	availableInstructionsInTrace++;
    	}
    	fseek(fp_in, currentPos, SEEK_SET); 

    	if (availableInstructionsInTrace < WIDTH) { // (1) fewer trace file
        	numInstructionsToFetch = availableInstructionsInTrace;
    	}
    	
    	int availableROBSpaces = ROB.availableSpace();
    	if (availableROBSpaces < numInstructionsToFetch) { // (2) fewer ROB spaces
        	numInstructionsToFetch = availableROBSpaces;
    	}
    	
    	for (int i = 0; i < numInstructionsToFetch; ++i) { // fetch to DE
        	char line[512];
        	if (fgets(line, sizeof(line), fp_in)) {
            		Instruction inst = Instruction::parseInstruction(line);
            		DE.push(inst);
        	} else {
            		break;
        	}
    	}
        
        for (int i = 0; i < numInstructionsToFetch; ++i) { // allocate to ROB
        if (!DE.isEmpty()) {
            ROB.enqueue(DE.pop());
        }
    }
}

void decode() {
 // If DE contains a decode bundle:
 // If RN is not empty (cannot accept a new rename bundle), then
 // do nothing. If RN is empty (can accept a new rename bundle),
 // then advance the decode bundle from DE to RN.
	if (RN.isEmpty()) {  
    		while (!DE.isEmpty()) {
        		RN.push(DE.pop());  
    		}
	}
}

void initializeRenameStructures() {
    for (int i = 0; i < NUM_ARCH_REGS; i++) {
        maptable[i] = MaptableEntry(i, true);  
    }
    for (int i = NUM_ARCH_REGS; i < NUM_PHYS_REGS; i++) {
        freelist.push(i);  
        maptable[i] = MaptableEntry(i, true); 
    }
}

void rename() {
 // If RN contains a rename bundle:
 // If either DI is not empty (cannot accept a new register-read
 // bundle) then do nothing. If DI is empty (can accept a new
 // dispatch bundle), then process (see below) the rename
 // bundle and advance it from RN to DI.
 //
 // How to process the rename bundle:
 // Apply your learning from the class lectures/notes on the
 // steps for renaming:
 // (1) Rename its source registers, and
 // (2) Rename its destination register (if it has one). If you
 // are not sure how to implement the register renaming, apply
 // the algorithm that you’ve learned from lectures and notes.
 // Note that the rename bundle must be renamed in program
 // order. Fortunately, the instructions in the rename bundle
 // are in program order).
 	if (!RN.isEmpty()) {
 		if (DI.isEmpty()) {
 			// process the rename bundle
 			// if src == -1 -> no src, else rename to p-reg in order (p1~)
 			// if des == -1 -> no dest, else rename to free p-reg, update ROB
 			// if src is updated by last des, update src
 			while (!RN.isEmpty()) {
                		Instruction inst = RN.pop();
                		if (inst.src1Reg != -1) {
                    			inst.src1Reg = maptable[inst.src1Reg].physReg;
                		}
                		if (inst.src2Reg != -1) {
                    			inst.src2Reg = maptable[inst.src2Reg].physReg;
                		}
                		if (inst.destReg != -1) {
                			if (freelist.empty()) {
                       				throw std::runtime_error("Free List is empty! Cannot allocate physical registers.");
                    			}
                    			
                			int newPhysReg = freelist.front();
                    			freelist.pop();
                    			
                    			freelist.push(maptable[inst.destReg].physReg);
                    			
                    			maptable[inst.destReg] = MaptableEntry(newPhysReg, true);
                   		 	inst.destReg = newPhysReg;  
                		}
                		
                		DI.push(inst);
                	}
 		}
 	}
}

void dispatch() {
 // If DI contains a dispatch bundle:
 // If the number of free IQ entries is less than the size of
 // the dispatch bundle in DI, then do nothing. If the number of
 // free IQ entries is greater than or equal to the size of the
 // dispatch bundle in DI, then dispatch all instructions from
 // DI to the IQ.
 	int BdayCounter = 0;
 	bool src1Ready = false;
 	bool src2Ready = false; 
	if (IQ.freeEntries>=DI.size()) {
		while (!DI.isEmpty()) {
 			// dispatch process
 			Instruction inst = DI.pop();
 			
 			if (maptable[inst.src1Reg].ready) {
 				src1Ready = true;
 				maptable[inst.src1Reg].ready = false;
 			}
 			if (maptable[inst.src2Reg].ready) {
 				src2Ready = true;
 				maptable[inst.src2Reg].ready = false;
 			}
 			maptable[inst.destReg].ready = false;
 			
 			IQ.enqueue(inst, src1Ready, src2Ready, BdayCounter);
 			BdayCounter++;
    		}
	}
}

void issue() {
 // Issue up to WIDTH oldest instructions from the IQ. (One
 // approach to implement oldest-first issuing is to make
 // multiple passes through the IQ, each time finding the next
 // oldest ready instruction and then issuing it. One way to
 // annotate the age of an instruction is to assign an
 // incrementing sequence number to each instruction as it is
 // fetched from the trace file.)
 //
 // To issue an instruction:
 // 1) Remove the instruction from the IQ.
 // 2) Wakeup dependent instructions (set their source operand
 // ready flags) in the IQ, so that in the next cycle IQ should
 // properly handle the dependent instructions.
	int issuedCount = 0;
	
	for (auto it = IQ.begin(); it != IQ.end() && issuedCount < WIDTH;) {
        	Instruction inst = *it;
        	
        	if (maptable[inst.src1Reg].ready && maptable[inst.src2Reg].ready) {
        		maptable[inst.destReg].ready = false;
        		IQ.dequeue(inst);
        		RR.push(inst);
        		issuedCount++;
        		
        		// wake up dependent inst
        		for (auto &iqInst : IQ) {
                	if (iqInst.src1Reg == inst.destReg) {
                    		iqInst.src1Ready = true;
                	}
                	if (iqInst.src2Reg == inst.destReg) {
                    		iqInst.src2Ready = true;
                	}
            		}
        	} 
        	else {
            		++it; // check next inst
        	}
        }
        	
}

void regRead() {
 // If RR contains a register-read bundle:
 // then process (see below) the register-read bundle up to
 // WIDTH instructions and advance it from RR to execute_list.
 //
 // How to process the register-read bundle:
 // Since values are not explicitly modeled, the sole purpose of
 // the Register Read stage is to pass the information of each
 // instruction in the register-read bundle (e.g. the renamed
 // source operands and operation type).
 // Aside from adding the instruction to the execute_list, set a
 // timer for the instruction in the execute_list that will
 // allow you to model its execution latency.
 	int processedCount = 0;
 	
 	for (auto it = RR.begin(); it != RR.end() && processedCount < WIDTH;) {
 		Instruction inst = *it;
 		
 		IQ.dequeue(inst);
 		execute_list.push(inst);
 		
 		processedCount++;
 	}
}

void execute() {
 // From the execute_list, check for instructions that are
 // finishing execution this cycle, and:
 // 1) Remove the instruction from the execute_list.
 // 2) Add the instruction to WB.
     if (!execute_list.isEmpty()) {
        Instruction inst = execute_list.front();
        execute_list.pop();
        // 연산 실행 (간단한 덧셈 가정)
        if (inst.opType == "ADD") {
            registers[inst.dest] = registers[inst.src1] + registers[inst.src2];
        }
        
        WB.push(inst);
        execute_list.pop(inst);
}

void writeback() {
 // Process the writeback bundle in WB: For each instruction in
 // WB, mark the instruction as “ready” in its entry in the ROB.
     if (!WB.isEmpty()) {
        Instruction inst = WB.front();
        WB.pop();
        
        // ROB ready
        ROB[inst.id].ready = true;
}

void commit() {
 // Commit up to WIDTH consecutive “ready” instructions from
 // the head of the ROB. Note that the entry of ROB should be
 // retired in the right order.
 	int committedCount = 0;
    
    	while (!ROB.empty() && committedCount < WIDTH) {
        	Instruction inst = ROB.front();
        
        if (inst.ready) {
            	ROB.pop();
            	cout << "Committed instruction " << inst.id << endl;
            	committedCount++;
        } else {
            	break;
        }
    }
}

int main(int argc, char *argv[]) {
	FILE *fp_in;
	FILE *fp_out;

	if (argc != 3) {
	   fprintf(stderr, "Usage: scope <input-file> <output-file>\n");
	   exit(-1);
	}
	else {
	   fp_in = fopen(argv[1], "r");
	   if (!fp_in) {
	      fprintf(stderr, "Cannot open input file `%s', exiting...\n",
			argv[1]);
	      exit(-1);
	   }

	   FILE *fp_temp = fopen(argv[2], "r");
	   if (fp_temp) {
	      fprintf(stderr, "Output file `%s' already exists, exiting...\n",
			argv[2]);
	      exit(-1);
	   }

	   fp_out = fopen(argv[2], "w");
	   if (!fp_out) {
	      fprintf(stderr, "Cannot create output file `%s', exiting...\n",
			argv[2]);
	      exit(-1);
	   }
	}

	printline PL(fp_out);

	char line[512];
	
	initializeRenameStructures();

	do{
            while (fgets(line, 512, fp_in)) {
	        if (line[0] != '#')	// comments are preceded by '#' in first character
	            PL.print(line);
	    }
	    
	        commit();
    		writeback();
    		execute();
    		regRead();
    		issue();
    		dispatch();
    		rename();
    		decode();
    		fetch(fp_in);

        } while (advance_cycle());

	fclose(fp_in);
	fclose(fp_out);


	// Create an html web page for viewing output file with scroll bars.
	//create_html(argv[2]);
}
