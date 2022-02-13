
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
    CCombFilterIf *pCombFilterIf=0;

    CAudioFileIf *phAudioFile = 0;
    CAudioFileIf *phAudioFileOut = 0;
    std::fstream hOutputFile;
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
        sOutputFilePath =  "out.wav";
        DelayLength = std::stof(argv[2]);
        Gain = std::stof(argv[3]);
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
    /*
    hOutputFile.open(sOutputFilePath.c_str(), std::ios::out);
    if (!hOutputFile.is_open())
    {
        cout << "Text file open error!";
        CAudioFileIf::destroy(phAudioFile);
        return -1;
    }
    */
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
        hOutputFile.close();
        return -1;
    }
    if (ppfAudioData[0] == 0)
    {
        CAudioFileIf::destroy(phAudioFile);
        hOutputFile.close();
        return -1;
    }

    time = clock();

    //////////////////////////////////////////////////////////////////////////////
    
    CCombFilterIf::create(pCombFilterIf);
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
        
        /*
        for (int i = 0; i < iNumFrames; i++)
        {
            for (int c = 0; c < stFileSpec.iNumChannels; c++)
            {
                //hOutputFile << ppfAudioData[c][i] << "\t";
            }
            //hOutputFile << endl;
        }
        */
        //ppfAudioDataOut = ppfAudioData;
        
    }
    
    

    cout << "\nreading/writing done in: \t" << (clock() - time) * 1.F / CLOCKS_PER_SEC << " seconds." << endl;

    //////////////////////////////////////////////////////////////////////////////
    // clean-up (close files and free memory)
    phAudioFileOut->closeFile();
    CAudioFileIf::destroy(phAudioFile);
    hOutputFile.close();

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

