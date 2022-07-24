#include <string>
#include <vector>
#include <iostream>
#include "stdlib.h"

#include "FPGA.hpp"
#include "reg_map.h"
#include "web_form_map.h"
#include "Socod.hpp"

    Socod::Socod()
    {
    	this->fpga = new FPGA();
        setChannelFrameNum();
    	acquiredData = new std::vector<int>(channelNum*frameNum);
    }

    Socod::~Socod() {
    	//delete fpga;
    	delete acquiredData;
    }

	reg_t Socod::getSelectChip(reg_t numChip){
		return numChip;
	}

    reg_t Socod::getChannelNum() {
    	return channelNum;
    }

    reg_t Socod::getFrameNum() {
    	return frameNum;
    }

    reg_t Socod::readReg(reg_t regNum) {
    	return fpga->readReg(regNum);
    }

    int Socod::writeReg(reg_t regNum, reg_t dataIn) {
    	return fpga->writeReg(regNum, dataIn);
    }

    bool Socod::setArmTrigger(bool armTrigger) {
    	fpga->writeRegWithMask(_DMX_REG_COMMAND, _DMX_REG_COMMAND_ARM_TRIGGER, armTrigger);
    	return armTrigger;
    }

    bool Socod::setForcedTrigger(bool forcedTrigger) {
    	fpga->writeRegWithMask(_DMX_REG_COMMAND, _DMX_REG_COMMAND_FORCED_TRIGGER, forcedTrigger);
    	return forcedTrigger;
    }

    void Socod::setStop(bool stop) {
    	fpga->writeRegWithMask(_DMX_REG_COMMAND, _DMX_REG_COMMAND_STOP, stop);
    	this->stop = stop;
    }

    bool Socod::isStop() {
    	return stop;
    }

	reg_t Socod::setGlobalLimit(reg_t glimit, reg_t chipnum){
		fpga->writeReg(_DMX_REG_NUM_POTS_SETUP_CH, chipnum);
		fpga->writeReg(_DMX_REG_NUM_POTS_SETUP_VAL, glimit);
		fpga->writeReg(_DMX_REG_COMMAND, 0x0040);
		//fpga->writeReg(_DMX_REG_COMMAND, 0x0080);
		return glimit;
	}

	reg_t Socod::setLocalReg(reg_t ilimit, reg_t daccode){
		fpga->writeReg(_DMX_REG_NUM_DAC_SETUP_CH, daccode);
		fpga->writeReg(_DMX_REG_NUM_DAC_SETUP_VAL, ilimit);
		fpga->writeReg(_DMX_REG_COMMAND, 0x0010);
		//fpga->writeReg(_DMX_REG_COMMAND, 0x0020);
		return ilimit;
	}

	void Socod::setControlReg(reg_t cntrnum){
		fpga->writeReg(_DMX_REG_COMMAND, cntrnum);
	}

	reg_t Socod::getDACChip(reg_t numChip){
        if (numChip == 1){return 0x0000;};
        if (numChip == 2){return 0x0020;};
        if (numChip == 3){return 0x0040;};
        if (numChip == 4){return 0x0060;};
        if (numChip == 5){return 0x0080;};
        if (numChip == 6){return 0x00A0;};

		if (numChip == 7){return 0x0100;}
		if (numChip == 8){return 0x0120;}
		if (numChip == 9){return 0x0140;}
		if (numChip == 10){return 0x0160;}
		if (numChip == 11){return 0x0180;}
		if (numChip == 12){return 0x01A0;}
    }


    reg_t Socod::getDACChannel (int numChannel, int numlimit){
        if ((numChannel == 1)&&(numlimit==2)){return 0x0000;};
        if ((numChannel == 1)&&(numlimit==1)){return 0x0001;};

        if ((numChannel == 2)&&(numlimit==2)){return 0x0002;};
        if ((numChannel == 2)&&(numlimit==1)){return 0x0003;};

        if ((numChannel == 3)&&(numlimit==4)){return 0x0004;};
        if ((numChannel == 3)&&(numlimit==3)){return 0x0005;};
        if ((numChannel == 3)&&(numlimit==2)){return 0x0006;};
        if ((numChannel == 3)&&(numlimit==1)){return 0x0007;};

        if ((numChannel == 4)&&(numlimit==4)){return 0x0008;};
        if ((numChannel == 4)&&(numlimit==3)){return 0x0009;};
        if ((numChannel == 4)&&(numlimit==2)){return 0x000A;};
        if ((numChannel == 4)&&(numlimit==1)){return 0x000B;};

        if ((numChannel == 5)&&(numlimit==2)){return 0x000C;};
        if ((numChannel == 5)&&(numlimit==1)){return 0x000D;};

        if ((numChannel == 6)&&(numlimit==2)){return 0x000E;};
        if ((numChannel == 6)&&(numlimit==1)){return 0x000F;};

        if ((numChannel == 7)&&(numlimit==4)){return 0x0010;};
        if ((numChannel == 7)&&(numlimit==3)){return 0x0011;};
        if ((numChannel == 7)&&(numlimit==2)){return 0x0012;};
        if ((numChannel == 7)&&(numlimit==1)){return 0x0013;};

        if ((numChannel == 8)&&(numlimit==4)){return 0x0014;};
        if ((numChannel == 8)&&(numlimit==3)){return 0x0015;};
        if ((numChannel == 8)&&(numlimit==2)){return 0x0016;};
        if ((numChannel == 8)&&(numlimit==1)){return 0x0017;};
    }

	reg_t Socod::setLocalLimit(reg_t numChip,int numChannel, int numlimit, reg_t ilimit){
		reg_t daccode=getDACChip(numChip)+getDACChannel(numChannel, numlimit);
		setLocalReg(ilimit, daccode);
		return ilimit;
	}

    reg_t Socod::setFrameLen(reg_t frameLen) {
    	fpga->writeReg(_DMX_REG_FRAME_LEN, frameLen);
    	return frameLen;
    }

    reg_t Socod::getFrameLen() {
    	reg_t toReturn;
    	fpga->readReg(_DMX_REG_FRAME_LEN, &toReturn);
    	return toReturn;
   }

    void Socod::setChannelFrameNum() {
        reg_t version_hi = 0;
        reg_t version_lo = 0;
        fpga->readReg(_DMX_REG_INT_VERSION_HI, &version_hi);
        fpga->readReg(_DMX_REG_INT_VERSION_LO, &version_lo);
        unsigned long full_version = (version_hi << 16) | version_lo;
        switch(full_version) {
            case 0xDE310001:
            	channelNum = 96;
            	frameNum = 4;
            	break;
            default:
            	channelNum = 96;
            	frameNum = 4;
            	break;
        }
    }

    reg_t Socod::getStatus() {
    	reg_t status;
    	fpga->readReg(_DMX_REG_STATUS, &status);
    	return status;
    }

    bool Socod::isDataReady() {
    	return (getStatus() & _DMX_REG_STATUS_ADC_DATA_READY);
    }

    bool Socod::isTriggerArmed() {
    	return (getStatus() & _DMX_REG_STATUS_TRIGGER_ARMED);
    }

    bool Socod::isInProgress() {
    	return (getStatus() & _DMX_REG_STATUS_IN_PROGRESS);
    }

    reg_t Socod::setTriggerMode(reg_t tMode) {
    	fpga->writeReg(_DMX_REG_TRG_MODE, tMode);
    	return tMode;
   }

    int Socod::acquireData(bool forcedTrigger, std::string startupMode) {
    	setArmTrigger(true);
    	setForcedTrigger(forcedTrigger);

    	while (isTriggerArmed()) {
    	    if (stop) {
    			setStop(false);
    			return -1;
    		}
    	}

    	while (!isDataReady()) {}

    	int j,k;
    	acquiredData->clear();
    	for (j = 0; j < frameNum; j++)
    		for (k=0; k < channelNum; k++)
    			acquiredData->push_back(fpga->readExpValue(k, j));

    	return 0;
    }

    std::vector<int>* Socod::getAcquiredDataVector() {
    	return acquiredData;
    }

    void Socod::writeMem(unsigned short int memAddr, unsigned short int memData) {
    	fpga->writeMem(memAddr, memData);
    }

    unsigned short int Socod::readMem(unsigned long int memAddr) {
    	return fpga->readMem(memAddr);
    }

    
