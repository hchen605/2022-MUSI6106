
#include <iostream>
#include <ctime>

#include "MUSI6106Config.h"

#include "AudioFileIf.h"
#include "CombFilterIf.h"
#include "RingBuffer.h"
#include <sstream>
#include <string.h>

using std::cout;
using std::endl;

// local function declarations
void    showClInfo ();

int test_filtering(char inPath[], char outPath[], int block_length, int mode, float delay_length, float gain);

int test_1();
int test_2();
int test_3();
int test_4();

/////////////////////////////////////////////////////////////////////////////////
// main function
int main(int argc, char* argv[])
{
    std::string sInputFilePath,                 //!< file paths
                sOutputFilePath,
                sOutputAudPath;

    static const int kBlockSize = 1024;

    clock_t time = 0;

    float **ppfAudioData = 0;
    float **ppfAudioDataOut = 0;
    float fDelayLength;
    float fGain;
    int   fmode;
    int testResult;
    CCombFilterIf *pCombFilterIf=0;

    CAudioFileIf *phAudioFile = 0;
    CAudioFileIf *phAudioFileOut = 0;
    std::fstream hTestOutputFile;
    CAudioFileIf::FileSpec_t stFileSpec;
    
    showClInfo();

    //////////////////////////////////////////////////////////////////////////////
    // parse command line arguments
    if (argc < 2)
    {
        //test mode
        cout << "Missing audio input path!"<<endl;
        cout << "Running test mode!"<<endl;
        //sOutputFilePath = "test.txt";
        char inputPath[50] =        "data/sin_400Hz.wav";
        char inputPath_zero[50] =   "data/zero.wav";
        char outputPath_1[50] =     "data/sin_400Hz_out_1.wav";
        char outputPath_2[50] =     "data/sin_400Hz_out_2.wav";
        char outputPath_3[50] =     "data/sin_400Hz_out_3.wav";
        char outputPath_4[50] =     "data/sin_400Hz_out_4.wav";
        char outputPath_5[50] =     "data/sin_400Hz_out_5.wav";
        //int  bk_size[] = {2048,512,256,128};
        int test;
        int testResult = 0;
        
        //test 1
        testResult = testResult + test_filtering(inputPath, outputPath_1, 1024, 1, 0.00125, 1);//400Hz half period
        //test 2
        testResult = testResult + test_filtering(inputPath, outputPath_2, 1024, 1, 0.0025, 1);//400Hz
        //test 3
        testResult = testResult + test_filtering(inputPath, outputPath_3, 2048, 1, 0.2, 1);
        //test 4
        testResult = testResult + test_filtering(inputPath_zero, outputPath_4, 1024, 1, 0.2, 1);
        //test 5 check zero gain, should sound the same as input
        testResult = testResult + test_filtering(inputPath, outputPath_5, 1024, 1, 0.2, 0);
        
        if(testResult==5)//may find another method to verify all pass
            cout<<"Finish running tests! You may check the restored test output files"<<endl;
        else
            cout<<"Test failed"<<endl;
                                
        //clean
                                
        return 0;
    }
    else
    {
        //application mode
        sInputFilePath = argv[1];
        sOutputAudPath =  argv[2];
        fmode = std::stoi(argv[3]);
        fDelayLength = std::stof(argv[4]);
        fGain = std::stof(argv[5]);
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
    phAudioFileOut->openFile(sOutputAudPath, CAudioFileIf::kFileWrite, &stFileSpec);
    if (!phAudioFileOut->isOpen())
    {
        cout << "Wave file open error!";
        CAudioFileIf::destroy(phAudioFileOut);
        return -1;
    }
    
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
    if (ppfAudioDataOut[0] == 0)
    {
        CAudioFileIf::destroy(phAudioFileOut);
        hTestOutputFile.close();
        return -1;
    }

    time = clock();

    //////////////////////////////////////////////////////////////////////////////
    //create filter object and decide which filter
    CCombFilterIf::create(pCombFilterIf);
    if(fmode == 1)
        pCombFilterIf->init(CCombFilterIf::kCombFIR, fDelayLength, stFileSpec.fSampleRateInHz, stFileSpec.iNumChannels);
    else
        pCombFilterIf->init(CCombFilterIf::kCombIIR, fDelayLength, stFileSpec.fSampleRateInHz, stFileSpec.iNumChannels);
    pCombFilterIf->setParam(CCombFilterIf::kParamGain, fGain);
    pCombFilterIf->setParam(CCombFilterIf::kParamDelay, fDelayLength);
    
    // get audio data, filter, and write it to the output
    while (!phAudioFile->isEof())
    {
        // set block length variable
        long long iNumFrames = kBlockSize;

        // read data (iNumOfFrames might be updated!)
        phAudioFile->readData(ppfAudioData, iNumFrames);
        cout << "\r" << "reading and filtering"<<endl;
        // filter
        pCombFilterIf->process(ppfAudioData, ppfAudioDataOut, iNumFrames);
        cout << "\r" << "Write audio"<<endl;
        phAudioFileOut->writeData(ppfAudioDataOut, iNumFrames);
        
    }
    
    

    cout << "\nreading/writing done in: \t" << (clock() - time) * 1.F / CLOCKS_PER_SEC << " seconds." << endl;
    
    
    
    //////////////////////////////////////////////////////////////////////////////
    // clean-up (close files and free memory)
    phAudioFileOut->closeFile();
    CAudioFileIf::destroy(phAudioFile);
    CAudioFileIf::destroy(phAudioFileOut);
    hTestOutputFile.close();

    for (int i = 0; i < stFileSpec.iNumChannels; i++){
        delete[] ppfAudioData[i];
        delete[] ppfAudioDataOut[i];
    }
    delete[] ppfAudioData;
    delete[] ppfAudioDataOut;
    ppfAudioData = 0;
    ppfAudioDataOut = 0;

    // all done
    return 0;

}//main

int test_filtering(char inPath[], char outPath[], int block_length, int mode, float delay_length, float gain)
{
    ///////////////////////////
    std::string sInputFilePath,                 //!< file paths
                sOutputFilePath,
                sOutputAudPath;
    int iTest = 0;
    int kBlockSize;
    //Testing
    //1/2. test simple sine wave
    sInputFilePath = inPath;
    sOutputAudPath = outPath;
    kBlockSize = block_length;
    
    //cout<<sOutputFilePath<<endl;
    
    float **ppfAudioData = 0;
    float **ppfAudioDataOut = 0;
    CCombFilterIf *pCombFilterIf=0;
    
    CAudioFileIf *phAudioFile = 0;
    CAudioFileIf *phAudioFileOut = 0;
    CAudioFileIf::FileSpec_t stFileSpec;
    
    float fDelayLength = delay_length;//400Hz
    float fGain = gain;
    float fMaxDelayLengthInS = 1;
    
    // open the input wave file
    CAudioFileIf::create(phAudioFile);
    phAudioFile->openFile(sInputFilePath, CAudioFileIf::kFileRead);
    if (!phAudioFile->isOpen())
    {
        cout << "testWave file open error!";
        CAudioFileIf::destroy(phAudioFile);
        return -1;
    }
    phAudioFile->getFileSpec(stFileSpec);
    
    //output observation: 1. should be 0s
    //create output file
    CAudioFileIf::create(phAudioFileOut);
    phAudioFileOut->openFile(sOutputAudPath, CAudioFileIf::kFileWrite, &stFileSpec);
    if (!phAudioFileOut->isOpen())
    {
        cout << "testWave file open error!";
        CAudioFileIf::destroy(phAudioFileOut);
        return -1;
    }
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
        return -1;
    }
    if (ppfAudioDataOut[0] == 0)
    {
        CAudioFileIf::destroy(phAudioFileOut);
        return -1;
    }
    
    //////////////////////////////////////////////////////////////////////////////
    //create filter object and decide which filter
    CCombFilterIf::create(pCombFilterIf);
    if(mode == 1)
        pCombFilterIf->init(CCombFilterIf::kCombFIR, fDelayLength, stFileSpec.fSampleRateInHz, stFileSpec.iNumChannels);
    else
        pCombFilterIf->init(CCombFilterIf::kCombIIR, fDelayLength, stFileSpec.fSampleRateInHz, stFileSpec.iNumChannels);
    pCombFilterIf->setParam(CCombFilterIf::kParamGain, fGain);
    pCombFilterIf->setParam(CCombFilterIf::kParamDelay, fDelayLength);
    
    // get audio data, filter, and write it to the output
    while (!phAudioFile->isEof())
    {
        // set block length variable
        long long iNumFrames = kBlockSize;

        // read data (iNumOfFrames might be updated!)
        phAudioFile->readData(ppfAudioData, iNumFrames);
        //cout << "\r" << "reading and filtering"<<endl;
        // filter
        pCombFilterIf->process(ppfAudioData, ppfAudioDataOut, iNumFrames);
        //cout << "\r" << "Write audio"<<endl;
        phAudioFileOut->writeData(ppfAudioDataOut, iNumFrames);
        
    }
    cout << "\r" << "finish reading and filtering"<<endl;
    
    //////////////////////////////////////////////////////////////////////////////
    // clean-up (close files and free memory)
    phAudioFileOut->closeFile();
    CAudioFileIf::destroy(phAudioFile);
    CAudioFileIf::destroy(phAudioFileOut);

    for (int i = 0; i < stFileSpec.iNumChannels; i++){
        delete[] ppfAudioData[i];
        delete[] ppfAudioDataOut[i];
    }
    delete[] ppfAudioData;
    delete[] ppfAudioDataOut;
    ppfAudioData = 0;
    ppfAudioDataOut = 0;
    
    iTest = 1;
    return iTest;
}

void     showClInfo()
{
    cout << "MUSI6106 Assignment Executable" << endl;
    cout << "(c) 2014-2022 by Alexander Lerch" << endl;
    cout  << endl;

    return;
}

