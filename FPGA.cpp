#include <stdio.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "reg_map.h"
#include "FPGA.hpp"

	void FPGA::regIOInit() {
		int memfd;
	    off_t dev_base = REG_BASE_ADDRESS;

	    memfd = open("/dev/mem", O_RDWR | O_SYNC);
	    if (memfd == -1) {
	        printf("Can't open /dev/mem.\n");
	    }
	    printf("/dev/mem opened.\n");

	    reg_mapped_addr = mmap(0, REG_MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, memfd, dev_base);
	    if (reg_mapped_addr == (void *) -1) {
	        printf("Can't map the memory to user space.\n");
	    }
	}

	void FPGA::memIOInit() {
		int memfd;
	    off_t dev_base = MEM_BASE_ADDRESS;

	    memfd = open("/dev/mem", O_RDWR | O_SYNC);
	    if (memfd == -1) {
	        printf("Can't open /dev/mem.\n");
	    }
	    printf("/dev/mem opened.\n");

	    mem_mapped_addr = mmap(0, MEM_MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, memfd, dev_base);
	    if (mem_mapped_addr == (void *) -1) {
	        printf("Can't map the memory to user space.\n");
	    }
	}

	void FPGA::write(void* addr, reg_t value) {
		*((volatile reg_t *) (addr)) = value;
	}

	reg_t FPGA::read(void* addr) {
		return *((volatile reg_t *) (addr));
	}

	void FPGA::writeUl(void* addr, unsigned long value) {
		*((volatile unsigned long *) (addr)) = value;
	}

	int FPGA::readIntWithBias(unsigned long int addr) {
		return *((volatile int *) (mem_mapped_addr + addr));
	}

	int FPGA::readInt(void* addr) {
		return *((volatile int *) (addr));
	}

	void FPGA::setRegNum(reg_t regNum) {
		write(reg_mapped_addr + REG_NUM_OFFSET, regNum);
	}

	void FPGA::setData(reg_t dataIn) {
		write(reg_mapped_addr + DATA_IN_OFFSET, dataIn);
	}

	void FPGA::setWE() {
		writeUl(reg_mapped_addr + REG_WE_OFFSET, W_EN);
	}

	reg_t FPGA::getData() {
		return read(reg_mapped_addr + DATA_OUT_OFFSET);
	}

    void FPGA::writeMem(unsigned short int memAddr, unsigned short int memData) {
    	write((void*)memAddr, memData);
    }

    unsigned short int FPGA::readMem(unsigned long int memAddr) {
    	return readIntWithBias(memAddr);
    }

	FPGA::FPGA() {
		regIOInit();
		memIOInit();
	}

	FPGA::~FPGA() {
		close(regfd);
		close(memfd);
	}

	int FPGA::readExpValue(int chNum, int frNum) {
		int res = (*((volatile int *) (mem_mapped_addr + MEM_FRAME_RANGE * frNum + MEM_VALUE_SIZE * chNum)));
		return res;
	}

	int FPGA::writeReg(reg_t regNum, reg_t dataToWrite) {
		setRegNum(regNum);
		setData(dataToWrite);
		setWE();

		reg_t checkData = getData();
		if (checkData != dataToWrite)
			return -1;
		return 0;
	}
	int FPGA::readReg(reg_t regNum, reg_t *dataToRead) {
		setRegNum(regNum);
		*dataToRead = getData();
		return 0;
	}

	int FPGA::readReg(reg_t regNum) {
		setRegNum(regNum);
		return getData();
	}

	int FPGA::readRegWithMask(reg_t regNum, reg_t *dataToRead, reg_t mask) {
		readReg(regNum, dataToRead);
		reg_t dataOut = *dataToRead;
		dataOut &= mask;
		*dataToRead = dataOut;
		return 0;
	}

	int FPGA::writeRegWithMask(reg_t regNum, reg_t dataToWrite, bool setData) {
		reg_t prevData;
		readReg(regNum, &prevData);
		if(setData)
			prevData |= dataToWrite;
		else
			prevData &= (~dataToWrite);
		writeReg(regNum, prevData);
		return 0;
	}



