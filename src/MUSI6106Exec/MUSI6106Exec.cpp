
#include <iostream>
#include <ctime>

#include "MUSI6106Config.h"

#include "AudioFileIf.h"
#include "CombFilterIf.h"
#include "RingBuffer.h"

using std::cout;
using std::endl;

// local function declarations
void    showClInfo ();

/////////////////////////////////////////////////////////////////////////////////
// main function
int main(int argc, char* argv[])
{
    std::string sInputFilePath,                 //!< file paths
                sOutputFilePath;

    static const int kBlockSize = 1024;

    clock_t time = 0;

    float **ppfAudioData = 0;
    float **ppfAudioDataOut = 0;
    float DelayLength;
    float Gain;
    int   Mode;
    CCombFilterIf *pCombFilterIf=0;

    CAudioFileIf *phAudioFile = 0;
    CAudioFileIf *phAudioFileOut = 0;
    std::fstream hTestOutputFile;
    CAudioFileIf::FileSpec_t stFileSpec;
    //CRingBuffer<float>* pCRingBuff = 0;
    //CCombFilterIf<float>* pCombFilter = 0;

    showClInfo();

    //////////////////////////////////////////////////////////////////////////////
    // parse command line arguments
    if (argc < 2)
    {
        cout << "Missing audio input path!";
        return -1;
    }
    else
    {
        sInputFilePath = argv[1];
        sOutputFilePath =  argv[2];
        Mode = std::stof(argv[3]);
        DelayLength = std::stof(argv[4]);
        Gain = std::stof(argv[5]);
    }
    
    //////////////////////////////////////////////////////////////////////////////
    // open the input wave file
    CAudioFileIf::create(phAudioFile);
    phAudioFile->openFile(sInputFilePath, CAudioFileIf::kFileRead);
    if (!phAudioFile->isOpen())
    {
        cout << "Wave file open error!";
        CAudioFileIf::destroy(phAudioFile);
        return -1;
    }
    phAudioFile->getFileSpec(stFileSpec);
    
    //create output file
    CAudioFileIf::create(phAudioFileOut);
    phAudioFileOut->openFile(sOutputFilePath, CAudioFileIf::kFileWrite);
    if (!phAudioFile->isOpen())
    {
        cout << "Wave file open error!";
        CAudioFileIf::destroy(phAudioFile);
        return -1;
    }
    //////////////////////////////////////////////////////////////////////////////
    // open the output text file
    hTestOutputFile.open(sOutputFilePath.c_str(), std::ios::out);
    if (!hTestOutputFile.is_open())
    {
        cout << "Text file open error!";
        CAudioFileIf::destroy(phAudioFile);
        return -1;
    }
    
    //hOutputFile.open(sOutputFilePath.c_str(), std::ios::out);
    //////////////////////////////////////////////////////////////////////////////
    // allocate memory
    ppfAudioData = new float*[stFileSpec.iNumChannels];
    for (int i = 0; i < stFileSpec.iNumChannels; i++)
        ppfAudioData[i] = new float[kBlockSize];
    
    ppfAudioDataOut = new float*[stFileSpec.iNumChannels];
    for (int i = 0; i < stFileSpec.iNumChannels; i++)
        ppfAudioDataOut[i] = new float[kBlockSize];
    

    if (ppfAudioData == 0)
    {
        CAudioFileIf::destroy(phAudioFile);
        hTestOutputFile.close();
        return -1;
    }
    if (ppfAudioData[0] == 0)
    {
        CAudioFileIf::destroy(phAudioFile);
        hTestOutputFile.close();
        return -1;
    }

    time = clock();

    //////////////////////////////////////////////////////////////////////////////
    
    CCombFilterIf::create(pCombFilterIf);
    if(Mode == 1)
        pCombFilterIf->init(CCombFilterIf::kCombFIR, DelayLength, stFileSpec.fSampleRateInHz, stFileSpec.iNumChannels);
    else
        pCombFilterIf->init(CCombFilterIf::kCombIIR, DelayLength, stFileSpec.fSampleRateInHz, stFileSpec.iNumChannels);
    pCombFilterIf->setParam(CCombFilterIf::kParamGain, Gain);
    pCombFilterIf->setParam(CCombFilterIf::kParamDelay, DelayLength);
    // get audio data, filter, and write it to the output
    while (!phAudioFile->isEof())
    {
        // set block length variable
        long long iNumFrames = kBlockSize;

        // read data (iNumOfFrames might be updated!)
        phAudioFile->readData(ppfAudioData, iNumFrames);
        // filter
        pCombFilterIf->process(ppfAudioData, ppfAudioDataOut, iNumFrames);
        phAudioFileOut->writeData(ppfAudioDataOut,iNumFrames);
        cout << "\r" << "reading and filtering";
        
    }
    
    

    cout << "\nreading/writing done in: \t" << (clock() - time) * 1.F / CLOCKS_PER_SEC << " seconds." << endl;

    ///////////////////////////
    //Testing
    //1/2. test simple square wave
    float **ppfTestSignal = 0;
    float **ppfTestSignalOut = 0;
    
    ppfTestSignal = new float *[stFileSpec.iNumChannels];
    // init to 0
    for (int c = 0; c < stFileSpec.iNumChannels; c++) {
        ppfTestSignal[c] = new float[20]();
    }
    // period = 10
    int period = 10;
    //int cnt = 0;
    for (int cnt = 0; cnt < 20; cnt++){
        if (cnt < period){
            for (int c = 0; c < stFileSpec.iNumChannels; c++) {
                ppfTestSignal[c][cnt] = 1.0;
            }
        }
        else
            for (int c = 0; c < stFileSpec.iNumChannels; c++) {
                ppfTestSignal[c][cnt] = -1.0;
            }
    }
    pCombFilterIf->process(ppfAudioData, ppfAudioDataOut, 20);
    for (int i = 0; i < 20; i++)
    {
        for (int c = 0; c < stFileSpec.iNumChannels; c++)
        {
            hTestOutputFile << ppfAudioData[c][i] << "\t";
        }
        hTestOutputFile << endl;
    }
    //output observation: 1. should be 0s
    
    //Test 3
    
    
    //////////////////////////////////////////////////////////////////////////////
    // clean-up (close files and free memory)
    phAudioFileOut->closeFile();
    CAudioFileIf::destroy(phAudioFile);
    hTestOutputFile.close();

    for (int i = 0; i < stFileSpec.iNumChannels; i++){
        delete[] ppfAudioData[i];
        delete[] ppfAudioDataOut[i];
    }
    delete[] ppfAudioData;
    delete[] ppfAudioDataOut;
    ppfAudioData = 0;

    // all done
    return 0;

}


void     showClInfo()
{
    cout << "MUSI6106 Assignment Executable" << endl;
    cout << "(c) 2014-2022 by Alexander Lerch" << endl;
    cout  << endl;

    return;
}

