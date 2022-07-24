#ifndef SOCOD_HPP_
#define SOCOD_HPP_

#include <vector>
#include <string>

#include "FPGA.hpp"

class Socod
{
private:
	Socod *socod;
	FPGA *fpga;
	bool stop;
	reg_t channelNum;
	reg_t frameNum;
    
	std::vector<int>* acquiredData;

    reg_t getDACChip( reg_t numChip);
    reg_t getDACChannel (int numChannel, int ilimit);

public:
    Socod();

    ~Socod();

    reg_t getSelectChip(reg_t numChip);

    reg_t getChannelNum();

    reg_t getFrameNum();

    reg_t readReg(reg_t regNum);

    int writeReg(reg_t regNum, reg_t dataIn);

    bool setArmTrigger(bool armTrigger);

    bool setForcedTrigger(bool forcedTrigger);

    void setStop(bool stop);

    reg_t setGlobalLimit(reg_t glimit, reg_t chipnum);

    reg_t setLocalLimit( reg_t numChip, int numChannel, int numlimit, reg_t ilimit);

    reg_t setLocalReg(reg_t ilimit, reg_t daccode);
    
    void setControlReg(reg_t cntrnum);

    reg_t setFrameLen(reg_t frameLen);

    reg_t getFrameLen();

    void setChannelFrameNum();

    reg_t getStatus();

    bool isDataReady();

    bool isTriggerArmed();

    bool isInProgress();

    reg_t setTriggerMode(reg_t tMode);

    int acquireData(bool forcedTrigger, std::string startupMode);

    std::vector<int>* getAcquiredDataVector();

    bool isStop();

    void writeMem(unsigned short int memAddr, unsigned short int memData);

    unsigned short int readMem(unsigned long int memAddr);

};


#endif /* SOCOD_HPP_ */
