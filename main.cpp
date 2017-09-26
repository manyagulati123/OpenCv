

#include <getopt.h>
#include <stdio.h>

#include <opencv2/highgui/highgui.hpp>

#include <iostream>
#include <sstream>
#include <iomanip>
#include <cstdlib>
#include <exception>
#include <sys/time.h>
#include "Include/VimbaCPP.h"
#include "CameraGrabber.h"
#include "LastFrame.h"
#include "FFmpegOutput.h"

// _____________________________________________________________________________
void help() {
  
}

int width = 1000;
int height = 480;
int fps = 24;
std::string crf = "0";  // 0 means lossless. The higher the lossier.
std::string label = "";
bool showCameraInput = true;

// _____________________________________________________________________________
bool parseOpt(int argc, char* const argv[]) {
  while (true) {
    static const struct option longopts[] = {
      {"width", required_argument, 0, 'w'},
      {"height", required_argument, 0, 'h'},
      {"fps", required_argument, 0, 'f'},
      {"crf", required_argument, 0, 'c'},
      {"silent", no_argument, 0, 's'},
      {0, 0, 0, 0}
    };
    int c = getopt_long(argc, argv, "hw:h:f:c:s", longopts, NULL);
    if (c == -1) break;

    switch (c) {
     case 'w':
      width = std::atoi(optarg);
      break;
     case 'h':
      height = std::atoi(optarg);
      break;
     case 'f':
      fps = std::atoi(optarg);
      break;
     case 'c':
      crf = optarg;
      break;
     case 's':
      showCameraInput = false;
      break;
     case ':':
     case '?':
      help();
      return false;
    }
  }

  if (argc - optind != 1) {
    std::cout << "No output label given!" << std::endl;
    help();
    return false;
  }






  return true;
}


std::string generateFilename(size_t cameraIndex) {
  for (size_t runNr = 1; runNr < (size_t) - 1; ++runNr) {
    std::ostringstream fn;
timeval curTime;
gettimeofday(&curTime, NULL);
int milli = curTime.tv_usec / 1;
    fn << milli << "_run" << std::setfill('0') << std::setw(2) << runNr
      << "_camera" << std::setfill('0') << std::setw(2) << cameraIndex + 1
      << ".h264";
   
    std::string fnStr = fn.str();
    if (FILE *file = fopen(fnStr.c_str(), "r")) {
      fclose(file);
    } else {
      return fnStr;
    }
  }
  throw std::runtime_error("Couldn't generate the output filename!");
  return "";  
}


int main(int argc, char* argv[]) {


  std::string pixelFormat = "Mono8";

  int ret = EXIT_SUCCESS;
  
  AVT::VmbAPI::VimbaSystem& system = AVT::VmbAPI::VimbaSystem::GetInstance();
  try {

    
    FFmpegOutput::initFFmpeg();

    if (system.Startup() != VmbErrorSuccess)
      throw std::runtime_error("Could not start VimbaSystem!");

    // Discover cameras.
    AVT::VmbAPI::CameraPtrVector cameras;
    if (system.GetCameras(cameras) != VmbErrorSuccess)
      throw std::runtime_error("Could not get the camera list!");

    // At least one camera is required.
    std::cout << "Found " << cameras.size() << " Camera(s)." << std::endl;
    if (cameras.empty()) throw std::runtime_error("No cameras connected!");

    // Classes receiving the last camera frames.
    std::vector<LastFrame> lastFrames(cameras.size());

    std::vector<CameraGrabber> grabbers(cameras.size());
    std::vector<AVT::VmbAPI::IFrameObserverPtr> receivers(cameras.size());

    for (size_t i = 0; i < cameras.size(); ++i) {

      
      AVPixelFormat framePixFmtInAvCodec;
      int frameLineSize[3] = {0, 0, 0};
      
      if (pixelFormat == "Mono8") {
std::cout<<framePixFmtInAvCodec;
        framePixFmtInAvCodec = AV_PIX_FMT_GRAY8;
        frameLineSize[0] = width;
std::cout<<AV_PIX_FMT_GRAY8;
      } else {
        throw std::runtime_error("Don't know how to read in camera pixel "
            "format in ffmpeg!");
      }

      // Get the output name.
      std::string fn = generateFilename(i);
      std::cout << "Output file " << i + 1 << ": " << fn << std::endl;

      // Create the ffmpeg output observer.
      receivers[i] = AVT::VmbAPI::IFrameObserverPtr(
            new FFmpegOutput(
              fn, width, height, fps, crf, cameras[i],
              framePixFmtInAvCodec, frameLineSize, // Describes the received vimba frame.
              &lastFrames[i]
            )
        );

      // Initialize the camera controller.
      grabbers[i].init(cameras[i], pixelFormat, width, height, fps, receivers[i]);
    }



    
    for (size_t i = 0; i < grabbers.size(); ++i) 
grabbers[i].startAcquisition();

    if (showCameraInput) {
      std::cout << std::endl << "Recording. Press <ESC> to stop..." << std::endl;

      while (true) {
        for (size_t i = 0; i < lastFrames.size(); ++i) {
          cv::Mat frame;
          lastFrames[i].get(frame);
          if (frame.cols > 0 && frame.rows > 0) {
            char wndName[16];
            sprintf(wndName, "Camera%lu", i + 1);
            cv::imshow(wndName, frame);
          }
        }
char key= (char)cv::waitKey(30);

        if (key == 27) break;
      }
    } else {
      
      std::cout << std::endl << "Recording. Press <ENTER> to stop..." << std::endl;
      getchar();
    }

    
    std::cout << "Stopping..." << std::endl;
    for (size_t i = 0; i < cameras.size(); ++i) grabbers[i].stopAcquisition();
  } catch (std::exception& e) {
    std::cout << "Error: " << e.what() << std::endl;
    ret = EXIT_FAILURE;
  }

 
  FFmpegOutput::closeFFmpeg();
  system.Shutdown();

  return ret;
}
