//
//  CombFilter.cpp
//  CombFilter
//
//  Created by Hsin-Hung Chen on 2022/2/11.
//
#include <iostream>
#include "CombFilter.h"

using std::cout;
using std::endl;

CCombFilterBase::CCombFilterBase(float fMaxDelayLengthInS, float fSampleRateInHz, int iNumChannels)
: mfGain(0.F),
iNumChannels(iNumChannels),
mfMaxDelayLengthInS(fMaxDelayLengthInS),
ppRingBuffer(nullptr),
mfSampleRateInHz(fSampleRateInHz)
{
    //setDelay(fMaxDelayLengthInS);
    iMaxDelayInSample = static_cast<int>(mfMaxDelayLengthInS * mfSampleRateInHz);
    //cout<<iMaxDelayInSample<<endl;
    //cout<<mfMaxDelayLengthInS<<endl;
    if (ppRingBuffer != nullptr) {
        delete ppRingBuffer;
    }
    ppRingBuffer = new CRingBuffer<float>*[iNumChannels];
    for (int i=0; i<iNumChannels; ++i)
        ppRingBuffer[i] = new CRingBuffer<float>(iMaxDelayInSample);
}

CCombFilterBase::~CCombFilterBase()
{
    delete ppRingBuffer;
}

/*
Error_t CCombFilterBase::setGain(float fGain) {
    
    mfGain = fGain;
    return Error_t::kNoError;
}
*/
/*! Update the delay length of filter. This would cause deleting & newing CRingBuffer object.
\param fDelay the delay time IN SECOND
*/
/*
Error_t CCombFilterBase::setDelay(float fDelay) {
    
    mfDelay = fDelay;
    iDelayInSample = static_cast<int>(mfDelay * mfSampleRateInHz);
    if (ppRingBuffer != nullptr) {
        delete ppRingBuffer;
    }
    ppRingBuffer = new CRingBuffer<float>*[iNumChannels];
    for (int i=0; i<iNumChannels; ++i)
        ppRingBuffer[i] = new CRingBuffer<float>(iDelayInSample);
    return Error_t::kNoError;
}
*/

Error_t CCombFilterBase::setParam(CCombFilterIf::FilterParam_t eParam, float fParamValue) {
    switch (eParam)
    {
    case CCombFilterIf::kParamGain:
        //return setGain(fParamValue);
        mfGain = fParamValue;
        break;
    case CCombFilterIf::kParamDelay:
        //return setDelay(fParamValue);
        mfDelay = fParamValue;
        iDelayInSample = static_cast<int>(mfDelay * mfSampleRateInHz);
        for (int i=0; i < iNumChannels; ++i)
            ppRingBuffer[i]->setWriteIdx(1.F*iDelayInSample + ppRingBuffer[i]->getReadIdx());
        break;
    
    case CCombFilterIf::kNumFilterParams:
        return Error_t::kFunctionInvalidArgsError;
    default:
        break;
    }
    return Error_t::kNoError;
}





float CCombFilterBase::getParam(CCombFilterIf::FilterParam_t eParam) const {
    switch (eParam)
    {
    case CCombFilterIf::kParamGain:
        //return getGain();
        return mfGain;
    case CCombFilterIf::kParamDelay:
        //return getDelay();
        return mfDelay;
    case CCombFilterIf::kNumFilterParams:
        return -1.F;
    default:
        break;
    }
    return -1.F;
}
 

FIRCombFilter::FIRCombFilter(float fMaxDelayLengthInS, float fSampleRateInHz, int iNumChannels)
    : CCombFilterBase(fMaxDelayLengthInS, fSampleRateInHz, iNumChannels) {}

Error_t FIRCombFilter::process(float** ppfInputBuffer, float** ppfOutputBuffer, int iNumberOfFrames) {
    for (int i = 0; i < iNumberOfFrames; ++i) {
        //ppRingBuffer->reset();
        //ppRingBuffer->setWriteIdx(iDelayInSample);
        for (int j = 0; j < iNumChannels; ++j) {
            //ppRingBuffer[j]->putPostInc(ppfInputBuffer[j][i]);
            ppfOutputBuffer[j][i] = ppfInputBuffer[j][i] + mfGain * ppRingBuffer[j]->getPostInc();
            ppRingBuffer[j]->putPostInc(ppfInputBuffer[j][i]);
        }
    }
    return Error_t::kNoError;
}

IIRCombFilter::IIRCombFilter(float fMaxDelayLengthInS, float fSampleRateInHz, int iNumChannels)
    : CCombFilterBase(fMaxDelayLengthInS, fSampleRateInHz, iNumChannels) {}

Error_t IIRCombFilter::process(float** ppfInputBuffer, float** ppfOutputBuffer, int iNumberOfFrames) {
    for (int i = 0; i < iNumberOfFrames; ++i) {
        //ppRingBuffer->reset();
        //ppRingBuffer->setWriteIdx(iDelayInSample);
        for (int j = 0; j < iNumChannels; ++j) {
            ppfOutputBuffer[j][i] = ppfInputBuffer[j][i] + mfGain * ppRingBuffer[j]->getPostInc();
            ppRingBuffer[j]->putPostInc(ppfOutputBuffer[j][i]);
        }
    }
    return Error_t::kNoError;
}
