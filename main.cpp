#include <cmath>
#include <iostream>

#include "Utility.hpp"
#include "Encoder.hpp"

void printHelp()
{
    std::cout << "===========================================" << std::endl;
    std::cout << "   cppeg - Simple JPEG Encoder Implemented by C++"    << std::endl;
    std::cout << "===========================================" << std::endl;
    std::cout << "Usage:\n" << std::endl;
    std::cout << "cppeg -h                              : Print this help message and exit" << std::endl;
    std::cout << "cppeg <iFile> [<oFile>]               : Compress a image denoted by <iFile> to a jpeg image."
                                                          "The name of the compressed image is determined by <oFile> if denoted." << std::endl;
}

void encodeJPEG(std::string iFilename, std::string oFilename="")
{
    
    std::cout << "Encoding..." << std::endl;
    
    // test for encdoer
    cppeg::Encoder encoder;

    if( encoder.open( iFilename, oFilename ))
    {
        if ( encoder.encodeImageFile() == cppeg::Encoder::ResultCode::ENCODE_DONE )
        {
            encoder.close();
            std::cout << "Complete! Check log file \'cppeg.log\' for details." << std::endl;
        }
    }
    else{
        std::cout << "Fail to open the files, unable to encode." << std::endl;
    }
}

int handleInput(int argc, char** argv)
{
    if ( argc < 2 )
    {
        std::cout << "No arguments provided." << std::endl;
        return EXIT_FAILURE;
    }
    
    if ( argc == 2 && (std::string)argv[1] == "-h" )
    {
        printHelp();
        return EXIT_SUCCESS;
    }
    else if ( argc == 2 )
    {
        encodeJPEG( argv[1] );
        return EXIT_SUCCESS;
    }
    else if ( argc == 3 )
    {
        encodeJPEG( argv[1], argv[2] );
        return EXIT_SUCCESS;
    }
    
    std::cout << "Incorrect usage, use -h to view help" << std::endl;
    return EXIT_FAILURE;
}

int main( int argc, char** argv )
{
    try
    {
        logFile << "lilbCPPEG - A simple JPEG encoder" << std::endl;
        
        return handleInput(argc, argv);
    }
    catch( std::exception& e )
    {
        std::cout << "Exceptions Occurred:-" << std::endl;
        std::cout << "What: " << e.what() << std::endl;
    }
    
    return EXIT_SUCCESS;
}