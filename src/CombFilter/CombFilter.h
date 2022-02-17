//
//  CombFilter.h
//  MUSI6106
//
//  Created by Hsin-Hung Chen on 2022/2/11.
//

#ifndef CombFilter_h
#define CombFilter_h
#include "CombFilterIf.h"
#include "RingBuffer.h"



class CCombFilterBase{
    
public:
    CCombFilterBase(float fMaxDelayLengthInS, float fSampleRateInHz, int iNumChannels);
    virtual ~CCombFilterBase();
    
    virtual Error_t process(float** ppfInputBuffer, float** ppfOutputBuffer, int iNumberOfFrames){

        return Error_t::kNoError;
    }
    
    Error_t setParam(CCombFilterIf::FilterParam_t eParam, float fParamValue);
    float getParam(CCombFilterIf::FilterParam_t eParam) const;
    
protected:
    CRingBuffer<float>** ppRingBuffer; //for multiple channel
    float                mfGain;
    float                mfDelay;
    float                mfSampleRateInHz;
    float                mfMaxDelayLengthInS;
    int                  iDelayInSample;
    int                  iMaxDelayInSample;
    int                  iNumChannels;
    
    
    
    
private:
    
    const float m_fMaxGain = 1.F;
    const float m_fMaxDelay = 1.F; // 1s
    //T* m_ptBuff;            //!< data buffer
};


class FIRCombFilter : public CCombFilterBase {
public:
    FIRCombFilter (float fMaxDelayLengthInS, float fSampleRateInHz, int iNumChannels);
    virtual ~FIRCombFilter(){};

    Error_t process(float** ppfInputBuffer, float** ppfOutputBuffer, int iNumberOfFrames) override;
};

class IIRCombFilter : public CCombFilterBase {
public:
    IIRCombFilter (float fMaxDelayLengthInS, float fSampleRateInHz, int iNumChannels);
    virtual ~IIRCombFilter(){};

    Error_t process(float** ppfInputBuffer, float** ppfOutputBuffer, int iNumberOfFrames) override;
};


#endif /* CombFilter_h */
