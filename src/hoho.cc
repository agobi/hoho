#ifdef  HAVE_CONFIG_H
#include "config.h"
#endif//HAVE_CONFIG_H

#include <RtAudio.h>
#include <iostream>
#include <fstream>
#include <cstdlib>

#include "synth.h"
#include "readfile.h"

unsigned int sampleRate = 48000;

int callback(void *outputBuffer, void *, unsigned int ,
        double , RtAudioStreamStatus status, void *userData)
{
  if ( status )
    std::cerr << "Stream underflow detected!" << std::endl;

  auto data = (synth::Player*)userData;

//  assert(nBufferFrames == data -> buffer_size);

  return data -> run((double *)outputBuffer);
}


void
help()
{
  std::cerr
    << "Usage: " << PACKAGE_NAME << " inputfile\n"
    << std::endl;
}

int
main (int argc, char *argv[])
{
  RtAudio dac;
  if (dac.getDeviceCount () < 1)
    {
      std::cout << "\nNo audio devices found!\n";
      exit (0);
    }
  RtAudio::StreamParameters parameters;
  parameters.deviceId = dac.getDefaultOutputDevice();
  parameters.nChannels = 2;
  parameters.firstChannel = 0;
  unsigned int bufferFrames = 0;

  if (argc < 2) {
    help();
    return 1;
  }

  synth::Patch patch;
  std::ifstream f(argv[1]);
  std::cout << "Start" <<std::endl;
  synth::read_file(f, patch);
  std::cout << "Start" <<std::endl;
  synth::Player player = synth::Player();


  try
  {
    dac.openStream (&parameters, NULL, RTAUDIO_FLOAT64,
      sampleRate, &bufferFrames, &callback, (void *)&player);

    patch.init_player(player, sampleRate, bufferFrames);


    dac.startStream ();
  }
  catch (RtAudioError & e)
  {
    e.printMessage ();
    exit (0);
  }

  char input;
  std::cout << "\nPlaying ... press <enter> to quit.\n";
  std::cin.get (input);
  try
  {
    // Stop the stream
    dac.stopStream ();
  }
  catch (RtAudioError & e)
  {
    e.printMessage ();
  }
  if (dac.isStreamOpen ())
    dac.closeStream ();
  return 0;
}
