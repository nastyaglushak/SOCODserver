/*
 * FPGA.hpp
 *
 *  Created on: May 14, 2018
 *      Author: learned_cat
 */

#ifndef FPGA_HPP_
#define FPGA_HPP_

#define DIMEX3_MEM_BASE_ADDR 0x300000

#define REG_MAP_SIZE 0x10000

#define REG_BASE_ADDRESS  0x43C00000
#define REG_WE_OFFSET     4*0
#define REG_NUM_OFFSET    4*1
#define DATA_IN_OFFSET    4*2
#define DATA_OUT_OFFSET   4*3

#define W_EN 0xFFFFFFFF

#define MEM_MAP_SIZE 0x80000

#define MEM_BASE_ADDRESS  0x40000000
#define MEM_FRAME_RANGE   4096
#define MEM_VALUE_SIZE    4

typedef unsigned short reg_t;

class FPGA
{
private:
	void* reg_mapped_addr;
	void* mem_mapped_addr;
	int memfd;
	int regfd;

	void regIOInit();

	void memIOInit();

	void write(void* addr, reg_t value);

	reg_t read(void* addr);

	void writeUl(void* addr, unsigned long value);

	int readInt(void* addr);

	void setRegNum(reg_t regNum);

	void setData(reg_t dataIn);

	void setWE();

	reg_t getData();

public:
	FPGA();

	~FPGA();

	int readExpValue(int chNum, int frNum);

	int writeReg(reg_t regNum, reg_t dataToWrite);

	int readReg(reg_t regNum, reg_t *dataToRead);

	int readReg(reg_t regNum);

	int readRegWithMask(reg_t regNum, reg_t *dataToRead, reg_t mask);

	int writeRegWithMask(reg_t regNum, reg_t dataToWrite, bool setData);

    void writeMem(unsigned short int memAddr, unsigned short int memData);

    unsigned short int readMem(unsigned long int memAddr);

    int readIntWithBias(unsigned long int addr);
};

#endif /* FPGA_HPP_ */
