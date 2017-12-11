// Disassembler.cpp : Defines the entry point for the console application.
//


/*
This is just a skeleton. It DOES NOT implement all the requirements.
It only recognizes the "ADD", "SUB" and "ADDI"instructions and prints
"Unkown Instruction" for all other instructions!

References:
(1) The risc-v ISA Manual ver. 2.1 @ https://riscv.org/specifications/
(2) https://github.com/michaeljclark/riscv-meta/blob/master/meta/opcodes
*/
#include "stdafx.h"
#include <iostream>
#include <fstream>
#include "stdlib.h"
#include <iomanip>
#include <string>

using namespace std;

int regs[32] = { 0 };
unsigned int pc = 0x0;

char memory[8 * 1024];    // only 8KB of memory located at address 0
ofstream outFile;

int BtoD(int num)
{
	int rem, dec = 0;
	int base = 1;
	while (num > 0)
	{
		rem = num % 10;
		dec = dec + rem * base;
		base = base * 2;
		num = num / 10;
	}

	return dec;
}

void emitError(char *s)
{
	cout << s;
	exit(0);
}

void printPrefix(unsigned int instA, unsigned int instW) {
	cout << "0x" << hex << std::setfill('0') << std::setw(8) << instA << "\t0x" << std::setw(8) << instW;
}
void instDecExec(unsigned int instWord)
{
	string print;
	unsigned int rd, rs1, rs2, funct3, funct7, opcode;
	unsigned int I_imm, S_imm, B_imm, U_imm, J_imm;
	unsigned int address;

	unsigned int instPC = pc - 4;

	opcode = instWord & 0x0000007F;
	rd = (instWord >> 7) & 0x0000001F;
	funct3 = (instWord >> 12) & 0x00000007;
	funct7 = (instWord >> 25) & 0x0000007F;
	rs1 = (instWord >> 15) & 0x0000001F;
	rs2 = (instWord >> 20) & 0x0000001F;

	// — inst[31] — inst[30:25] inst[24:21] inst[20]
	I_imm = ((instWord >> 20) & 0x7FF) | (((instWord >> 31) ? 0xFFFFF800 : 0x0));
	I_imm = ((instWord >> 20) & 0x7FF) | (((instWord >> 31) ? 0xFFFFF800 : 0x0));
	S_imm = (((instWord >> 7) & 0x0000001F) | ((instWord >> 25) & 0x0000003F) << 5 | ((instWord >> 31) ? 0xFFFFF800 : 0x0));
	B_imm = (((instWord >> 7) & 0x00000001) << 11 | ((instWord >> 8) & 0x0000000F) << 1 | ((instWord >> 25) & 0x0000003F) << 5 | ((instWord >> 31) ? 0xFFFFF800 : 0x0));
	U_imm = (instWord & 0xFFFFF000);
	J_imm = (((instWord >> 21) & 0x000003FF) << 1 | ((instWord >> 20) & 0x00000001) << 11 | ((instWord >> 12) & 0x000000FF) << 12 | ((instWord >> 30) & 0x00000001) << 19 | ((instWord >> 31) ? 0xFFFFF800 : 0x0));

	printPrefix(instPC, instWord);
	regs[0] = 0;
	if (opcode == 0x33) {        // R Instructions
		switch (funct3) {
		case 0:
			if (funct7 == 32)
			{
				print = "\tsub x" + to_string(rd) + ",x" + to_string(rs1) + ",x" + to_string(rs2) + "\n";
				cout << print;
				//cout << "\tSUB\tx" << rd << ",x" << rs1 << ",x" << rs2 << "\n";
				regs[rd] = regs[rs1] - regs[rs2];
			}
			else
			{
				print = "\tadd x" + to_string(rd) + ",x" + to_string(rs1) + ",x" + to_string(rs2) + "\n";
				cout << print;
				//cout << "\tADD\tx" << rd << ",x" << rs1 << ",x" << rs2 << "\n";
				regs[rd] = regs[rs1] + regs[rs2];
			}
			break;

		case 1:
			print = "\tsll x" + to_string(rd) + ",x" + to_string(rs1) + ",x" + to_string(rs2) + "\n";
			cout << print;
			//cout << "\tSLL\tx" << rd << ",x" << rs1 << ",x" << rs2 << "\n";
			regs[rd] = regs[rs1] << ((int)rs2 & 0x0000001F);
			break;
		case 2:

			print = "\tslt x" + to_string(rd) + ",x" + to_string(rs1) + ",x" + to_string(rs2) + "\n";
			cout << print;
			//cout << "\tSLT\tx" << rd << ",x" << rs1 << ",x" << rs2 << "\n";
			if (regs[rs1] < regs[rs2])
				regs[rd] = 1;
			else
				regs[rd] = 0;
			break;
		case 3:

			print = "\tsltu x" + to_string(rd) + ",x" + to_string(rs1) + ",x" + to_string(rs2) + "\n";
			cout << print;
			//cout << "\tSLTU\tx" << rd << ",x" << rs1 << ",x" << rs2 << "\n";
			if (unsigned int(regs[rs1]) < unsigned int(regs[rs2]))
				regs[rd] = 1;
			else
				regs[rd] = 0;
			break;
		case 4:

			print = "\txor x" + to_string(rd) + ",x" + to_string(rs1) + ",x" + to_string(rs2) + "\n";
			cout << print;
			//cout << "\tXOR\tx" << rd << ",x" << rs1 << ",x" << rs2 << "\n";
			regs[rd] = regs[rs1] ^ regs[rs2];
			break;
		case 5:
			if (funct7 == 0)
			{

				print = "\tsrl x" + to_string(rd) + ",x" + to_string(rs1) + ",x" + to_string(rs2) + "\n";
				cout << print;
				//cout << "\tSRL\tx" << rd << ",x" << rs1 << "," << hex << "0x" << rs2 << "\n";
				regs[rd] = regs[rs1] >> regs[rs2];
			}
			else
			{
				int x = regs[rs1] & 0x10000000;

				print = "\tsra x" + to_string(rd) + ",x" + to_string(rs1) + ",x" + to_string(rs2) + "\n";
				cout << print;
				//cout << "\tSRA\tx" << rd << ",x" << rs1 << "," << hex << "0x" << rs2 << "\n";
				regs[rd] = (regs[rs1] >> regs[rs2]) | x;
			}
			break;
		case 6:

			print = "\tor x" + to_string(rd) + ",x" + to_string(rs1) + ",x" + to_string(rs2) + "\n";
			cout << print;
			//cout << "\tOR\tx" << rd << ",x" << rs1 << "," << hex << "0x" << rs2 << "\n";
			regs[rd] = regs[rs1] | regs[rs2];
			break;
		case 7:

			print = "\tand x" + to_string(rd) + ",x" + to_string(rs1) + ",x" + to_string(rs2) + "\n";
			cout << print;
			//cout << "\tAND\tx" << rd << ",x" << rs1 << "," << hex << "0x" << rs2 << "\n";
			regs[rd] = regs[rs1] & regs[rs2];
			break;

		default:
			cout << "\tUnkown R Instruction \n";
		}
	}
	else if (opcode == 0x13) {    // I instructions
		switch (funct3) {
		case 0:

			print = "\taddi x" + to_string(rd) + ",x" + to_string(rs1) + "," + to_string((int)I_imm) + "\n";
			cout << print;
			//cout << "\tADDI\tx" << rd << ",x" << rs1 << "," << hex << "0x" << (int)I_imm << "\n";
			regs[rd] = regs[rs1] + (int)I_imm;
			break;
		case 1:

			print = "\tslli x" + to_string(rd) + ",x" + to_string(rs1) + "," + to_string((int)I_imm & 0x0000001F) + "\n";
			cout << print;
			//cout << "\tSLLI\tx" << rd << ",x" << rs1 << "," << hex << "0x" << ((int)I_imm & 0x0000001F) << "\n";
			regs[rd] = regs[rs1] << ((int)I_imm & 0x0000001F);
			break;

		case 5:
			if (funct7 == 0)
			{
				print = "\tsrli x" + to_string(rd) + ",x" + to_string(rs1) + "," + to_string((int)I_imm & 0x0000001F) + "\n";
				cout << print;
				//cout << "\tSRLI\tx" << rd << ",x" << rs1 << "," << hex << "0x" << ((int)I_imm & 0x0000001F) << "\n";
				regs[rd] = regs[rs1] >> ((int)I_imm & 0x0000001F);
			}
			else
			{
				int x = regs[rs1] & 0x10000000;

				print = "\tsrai x" + to_string(rd) + ",x" + to_string(rs1) + "," + to_string((int)I_imm & 0x0000001F) + "\n";
				cout << print;
				//cout << "\tSRAI\tx" << rd << ",x" << rs1 << "," << hex << "0x" << ((int)I_imm & 0x0000001F) << "\n";
				regs[rd] = (regs[rs1] >> regs[rs2]) | x;;
			}

			break;
		case 2:

			print = "\tslti x" + to_string(rd) + ",x" + to_string(rs1) + "," + to_string((int)I_imm) + "\n";
			cout << print;
			//cout << "\tSLTI\tx" << rd << ",x" << rs1 << "," << hex << "0x" << (int)I_imm << "\n";
			if (int(regs[rs1]) < int(I_imm))
				regs[rd] = 1;
			else
				regs[rd] = 0;
			break;
		case 3:
			print = "\tsltiu x" + to_string(rd) + ",x" + to_string(rs1) + "," + to_string((int)I_imm) + "\n";
			cout << print;
			//cout << "\tSLTIU\tx" << rd << ",x" << rs1 << "," << hex << "0x" << I_imm << "\n";
			if (regs[rs1] < I_imm)
				regs[rd] = 1;
			else
				regs[rd] = 0;
			break;
		case 4:
			print = "\txori x" + to_string(rd) + ",x" + to_string(rs1) + "," + to_string((int)I_imm) + "\n";
			cout << print;
			//cout << "\tXORI\tx" << rd << ",x" << rs1 << "," << hex << "0x" << (int)I_imm << "\n";
			regs[rd] = regs[rs1] ^ (int)I_imm;
			break;
		case 6:
			print = "\tori x" + to_string(rd) + ",x" + to_string(rs1) + "," + to_string((int)I_imm) + "\n";
			cout << print;
			//cout << "\tORI\tx" << rd << ",x" << rs1 << "," << hex << "0x" << (int)I_imm << "\n";
			regs[rd] = regs[rs1] | (int)I_imm;
			break;
		case 7:
			print = "\tandi x" + to_string(rd) + ",x" + to_string(rs1) + "," + to_string((int)I_imm) + "\n";
			cout << print;
			//cout << "\tANDI\tx" << rd << ",x" << rs1 << "," << hex << "0x" << (int)I_imm << "\n";
			regs[rd] = regs[rs1] & (int)I_imm;
			break;
		default:
			cout << "\tUnkown I Instruction \n";
		}
	}

	else if (opcode == 0x3)
	{
		switch (funct3)
		{
		case 0:

			print = "\tlb x" + to_string(rd) + "," + to_string((int)I_imm) + "(" + to_string(rs1) + ")" + "\n";
			cout << print;
			//cout << "\tLB\tx" << rd << ",x" << rs1 << "," << hex << "0x" << (int)I_imm << "\n";
			regs[rd] = memory[regs[rs1] + (int)I_imm];
			break;
		case 1:

			print = "\tlh x" + to_string(rd) + "," + to_string((int)I_imm) + "(" + to_string(rs1) + ")" + "\n";
			cout << print;
			//cout << "\tLH\tx" << rd << ",x" << rs1 << "," << hex << "0x" << (int)I_imm << "\n";
			regs[rd] = (memory[regs[rs1] + (int)I_imm]) | ((memory[regs[rs1] + (int)I_imm + 1] << 8));
			//STILL NEED TO SIGN EXTEND
			break;
		case 2:

			print = "\tlw x" + to_string(rd) + "," + to_string((int)I_imm) + "(" + to_string(rs1) + ")" + "\n";
			cout << print;
			//cout << "\tLW\tx" << rd << ",x" << rs1 << "," << hex << "0x" << (int)I_imm << "\n";
			regs[rd] = (memory[regs[rs1] + (int)I_imm]) | ((memory[regs[rs1] + (int)I_imm + 1] << 8))
				| ((memory[regs[rs1] + (int)I_imm + 2] << 16)) | ((memory[regs[rs1] + (int)I_imm + 3] << 24));
			break;
		case 4:

			print = "\tlbu x" + to_string(rd) + "," + to_string((int)I_imm) + "(" + to_string(rs1) + ")" + "\n";
			cout << print;
			//cout << "\tLBU\tx" << rd << ",x" << rs1 << "," << hex << "0x" << I_imm << "\n";
			regs[rd] = memory[regs[rs1] + I_imm];
			break;
		case 5:

			print = "\tlhu x" + to_string(rd) + "," + to_string((int)I_imm) + "(" + to_string(rs1) + ")" + "\n";
			cout << print;
			//cout << "\tLHU\tx" << rd << ",x" << rs1 << "," << hex << "0x" << I_imm << "\n";
			regs[rd] = memory[regs[rs1] + I_imm] | ((memory[regs[rs1] + I_imm + 1]) << 8);
			break;
		}
	}
	else  if (opcode == 0x63)
	{

		switch (funct3)
		{
		case 0:

			print = "\tbeq x" + to_string(rd) + ",x" + to_string(rs1) + "," + to_string((int)B_imm) + "\n";
			cout << print;
			//cout << "\tBEQ\tx" << rs1 << ",x" << rs2 << "," << hex << "0x" << (int)B_imm << "\n";
			if (regs[rs1] == regs[rs2])
				pc = pc - 4 + (int)B_imm;
			break;
		case 1:

			print = "\tbne x" + to_string(rd) + ",x" + to_string(rs1) + "," + to_string((int)B_imm) + "\n";
			cout << print;
			//cout << "\tBNE\tx" << rs1 << ",x" << rs2 << "," << hex << "0x" << (int)B_imm << "\n";
			if (regs[rs1] != regs[rs2])
				pc = pc - 4 + (int)B_imm;
			break;
		case 4:

			print = "\tblt x" + to_string(rd) + ",x" + to_string(rs1) + "," + to_string((int)B_imm) + "\n";
			cout << print;
			//cout << "\tBLT\tx" << rs1 << ",x" << rs2 << "," << hex << "0x" << (int)B_imm << "\n";
			if (regs[rs1] < regs[rs2])
				pc = pc - 4 + (int)B_imm;
			break;
		case 5:

			print = "\tbge x" + to_string(rd) + ",x" + to_string(rs1) + "," + to_string((int)B_imm) + "\n";
			cout << print;
			//cout << "\tBGE\tx" << rs1 << ",x" << rs2 << "," << hex << "0x" << (int)B_imm << "\n";
			if (regs[rs1] >= regs[rs2])
				pc = pc - 4 + (int)B_imm;
			break;
		case 6:

			print = "\tbltu x" + to_string(rd) + ",x" + to_string(rs1) + "," + to_string((int)B_imm) + "\n";
			cout << print;
			//cout << "\tBLTU\tx" << rs1 << ",x" << rs2 << "," << hex << "0x" << (int)B_imm << "\n";
			if ((unsigned int)regs[rs1] < (unsigned int)regs[rs2])
				pc = pc - 4 + (int)B_imm;
			break;
		case 7:

			print = "\tbgtu x" + to_string(rd) + ",x" + to_string(rs1) + "," + to_string((int)B_imm) + "\n";
			cout << print;
			//cout << "\tBGTU\tx" << rs1 << ",x" << rs2 << "," << hex << "0x" << (int)B_imm << "\n";
			if ((unsigned int)regs[rs1] >= (unsigned int)regs[rs2])
				pc = pc - 4 + (int)B_imm;
			break;

		}

	}
	else if (opcode == 0x67)
	{
		print = "\tJALR x" + to_string(rd) + "," + to_string((int)I_imm) + "\n";
		cout << print;

		//cout << "\tJALR\tx" << rd << "," << hex << "0x" << (int)I_imm << "\n";
		regs[rd] = pc;
		address = (int)I_imm + regs[rs1];
		pc = address & 0xFFFFFFFE;
	}

	else if (opcode == 0x6F)
	{
		print = "\tJAL x" + to_string(rd) + "," + to_string((int)J_imm) + "\n";
		cout << print;

		//cout << "\tJAL\tx" << rd << "," << hex << "0x" << (int)J_imm << "\n";
		regs[rd] = pc;
		pc = pc - 4 + (int)J_imm;
	}
	else if (opcode == 0x17)
	{
		print = "\tAUIPC x" + to_string(rd) + "," + to_string((int)U_imm) + "\n";
		cout << print;

		//cout << "\tAUIPC\tx" << rd << "," << hex << "0x" << (int)U_imm << "\n";
		address = (U_imm << 12) & 0xFFFFF000;
		regs[rd] = pc + address;

	}
	else if (opcode == 0x37)
	{
		print = "\tLUI x" + to_string(rd) + "," + to_string((int)U_imm) + "\n";
		cout << print;

		//cout << "\tLUI\tx" << rd << "," << hex << "0x" << (int)U_imm << "\n";
		regs[rd] = ((int)U_imm << 12) & 0xFFFFF000;
	}
	else if (opcode == 0x73)
	{
		if (regs[17] == 1)
		{
			cout << "Print Int : ";
			cout << BtoD(regs[10]);
		}
		else if (regs[17] == 4)
		{
			cout << "Print String : " << endl;
			string s = to_string(regs[10]);
			int length = s.length();
			string newString;

			for (int i = 0; i< length; i += 2)

			{
				string byte = s.substr(i, 2);
				char chr = (char)(int)strtol(byte.c_str(), NULL, 16);
				newString.push_back(chr);
			}

			cout << newString << endl;
		}
		else if (regs[17] == 5)
		{
			int x;
			cout << "Read Int : ";
			cin >> x;

			regs[17] = x;
		}
		else if (regs[17] = 10)
		{
			cout << "\nProgram Exit Successfully\n";

			system("pause");

			exit(EXIT_SUCCESS);
		}

	}
	else if (opcode == 0x23)
	{
		switch (funct3)
		{
		case 0:

			print = "\tsb x" + to_string(rd) + "," + to_string((int)S_imm) + "(" + to_string(rs1) + ")" + "\n";
			cout << print;

			//cout << "\tSB\tx" << rs1 << ",x" << rs2 << "," << hex << "0x" << (int)S_imm << "\n";
			memory[regs[rs1] + (int)S_imm] = regs[rs2];
			break;
		case 1:

			print = "\tsh x" + to_string(rd) + "," + to_string((int)S_imm) + "(" + to_string(rs1) + ")" + "\n";
			cout << print;
			//cout << "\tSH\tx" << rs1 << ",x" << rs2 << "," << hex << "0x" << (int)S_imm << "\n";
			memory[regs[rs1] + (int)S_imm] = (regs[rs2] & 0xFF);
			memory[regs[rs1] + (int)S_imm + 1] = ((regs[rs2] >> 8) & 0xFF);
			break;
		case 2:

			print = "\tsw x" + to_string(rd) + "," + to_string((int)S_imm) + "(" + to_string(rs1) + ")" + "\n";
			cout << print;
			//cout << "\tSW\tx" << rs1 << ",x" << rs2 << "," << hex << "0x" << (int)S_imm << "\n";
			memory[regs[rs1] + (int)S_imm] = regs[rs2];
			memory[regs[rs1] + (int)S_imm + 1] = ((regs[rs2] >> 8) & 0xFF);
			memory[regs[rs1] + (int)S_imm + 2] = ((regs[rs2] >> 16) & 0xFF);
			memory[regs[rs1] + (int)S_imm + 3] = ((regs[rs2] >> 24) & 0xFF);
			break;
		}
	}
	else {
		cout << "\tUnkown Instruction \n";
	}

	outFile.write(print.c_str(), print.length());

}

int main(int argc, char *argv[]) {

	unsigned int instWord = 0;
	ifstream inFile;
	outFile.open("sample.txt");


	if (argc<1) emitError("use: rv32i_sim <machine_code_file_name>\n");

	inFile.open(argv[1], ios::in | ios::binary | ios::ate);

	if (inFile.is_open())
	{
		int fsize = inFile.tellg();

		inFile.seekg(0, inFile.beg);
		if (!inFile.read((char *)memory, fsize)) emitError("Cannot read from input file\n");

		while (!inFile.eof()) {
			instWord = (unsigned char)memory[pc] |
				(((unsigned char)memory[pc + 1]) << 8) |
				(((unsigned char)memory[pc + 2]) << 16) |
				(((unsigned char)memory[pc + 3]) << 24);
			pc += 4;
			// remove the following line once you have a complete simulator
			//if (pc == 32) break;            // stop when PC reached address 32
			instDecExec(instWord);
		}

		// dump the registers
		for (int i = 0; i<32; i++)
			cout << "x" << dec << i << ": \t" << "0x" << hex << std::setfill('0') << std::setw(8) << regs[i] << "\n";

	}
	else emitError("Cannot access input file\n");

	outFile.close();
	system("pause");
	return 0;
}

