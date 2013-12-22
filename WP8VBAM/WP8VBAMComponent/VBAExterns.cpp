#include "VBAM/System.h"

int systemSaveUpdateCounter = SYSTEM_SAVE_NOT_UPDATED;
u32 systemColorMap32[0x10000];
u16 systemColorMap16[0x10000];
int emulating = 0;
int systemFrameSkip = 0;
int systemRedShift = 0;
int systemBlueShift = 0;
int systemGreenShift = 0;
int systemColorDepth;

void systemDrawScreen()
{
  //if(theApp.display == NULL)
  //  return;

  //theApp.renderedFrames++;

  //if(theApp.updateCount) {
  //  POSITION pos = theApp.updateList.GetHeadPosition();
  //  while(pos) {
  //    IUpdateListener *up = theApp.updateList.GetNext(pos);
  //    up->update();
  //  }
  //}

  //if (Sm60FPS_CanSkipFrame())
	 // return;

  //if( theApp.aviRecording ) {
	 // if( theApp.painting ) {
		//  theApp.skipAudioFrames++;
	 // } else {
		//  unsigned char *bmp;
		//  unsigned short srcPitch = theApp.sizeX * ( systemColorDepth >> 3 ) + 4;
		//  switch( systemColorDepth )
		//  {
		//  case 16:
		//	  bmp = new unsigned char[ theApp.sizeX * theApp.sizeY * 2 ];
		//	  cpyImg16bmp( bmp, pix + srcPitch, srcPitch, theApp.sizeX, theApp.sizeY );
		//	  break;
		//  case 32:
		//	  // use 24 bit colors to reduce video size
		//	  bmp = new unsigned char[ theApp.sizeX * theApp.sizeY * 3 ];
		//	  cpyImg32bmp( bmp, pix + srcPitch, srcPitch, theApp.sizeX, theApp.sizeY );
		//	  break;
		//  }
		//  if( false == theApp.aviRecorder->AddVideoFrame( bmp ) ) {
		//	  systemMessage( IDS_AVI_CANNOT_WRITE_VIDEO, "Cannot write video frame to AVI file." );
		//	  delete theApp.aviRecorder;
		//	  theApp.aviRecorder = NULL;
		//	  theApp.aviRecording = false;
		//  }
		//  delete [] bmp;
	 // }
  //}

  //if( theApp.ifbFunction ) {
	 // theApp.ifbFunction( pix + (theApp.filterWidth * (systemColorDepth>>3)) + 4,
		//  (theApp.filterWidth * (systemColorDepth>>3)) + 4,
		//  theApp.filterWidth, theApp.filterHeight );
  //}

  //if(!soundBufferLow)
  //{
	 // theApp.display->render();
  //    Sm60FPS_Sleep();
  //}
  //else
	 // soundBufferLow = false;

}

void systemScreenCapture(int captureNumber)
{
  /*if(theApp.m_pMainWnd)
    ((MainWnd *)theApp.m_pMainWnd)->screenCapture(captureNumber);*/
}

void systemShowSpeed(int speed)
{
  /*systemSpeed = speed;
  theApp.showRenderedFrames = theApp.renderedFrames;
  theApp.renderedFrames = 0;
  if(theApp.videoOption <= VIDEO_6X && theApp.showSpeed) {
    CString buffer;
    if(theApp.showSpeed == 1)
      buffer.Format(VBA_NAME_AND_SUBVERSION "-%3d%%", systemSpeed);
    else
      buffer.Format(VBA_NAME_AND_SUBVERSION "-%3d%%(%d, %d fps)", systemSpeed,
                    systemFrameSkip,
                    theApp.showRenderedFrames);

    systemSetTitle(buffer);
  }*/
}

int systemGetSensorX()
{
  return 0;
}

int systemGetSensorY()
{
  return 0;
}

void systemMessage(int x, const char *y, ...)
{

}

u32 systemReadJoypad(int which)
{
  
  return 0;
}

bool systemReadJoypads()
{
  
  return 0;
}

void systemFrame()
{

}

bool systemCanChangeSoundQuality()
{
  return true;
}

void systemOnWriteDataToSoundBuffer(const u16 * finalWave, int length)
{

}

void systemUpdateMotionSensor()
{

}

bool systemPauseOnFrame()
{
	return false;
}
void system10Frames(int rate)
{

//	if( theApp.autoFrameSkip )
//	{
//		u32 time = systemGetClock();
//		u32 diff = time - theApp.autoFrameSkipLastTime;
//		theApp.autoFrameSkipLastTime = time;
//		if( diff ) {
//			// countermeasure against div/0 when debugging
//			Sm60FPS::nCurSpeed = (1000000/rate)/diff;
//		} else {
//			Sm60FPS::nCurSpeed = 100;
//		}
//	}
//
//
//  if(theApp.rewindMemory) {
//    if(++theApp.rewindCounter >= (theApp.rewindTimer)) {
//      theApp.rewindSaveNeeded = true;
//      theApp.rewindCounter = 0;
//    }
//  }
//  if(systemSaveUpdateCounter) {
//    if(--systemSaveUpdateCounter <= SYSTEM_SAVE_NOT_UPDATED) {
//      ((MainWnd *)theApp.m_pMainWnd)->writeBatteryFile();
//      systemSaveUpdateCounter = SYSTEM_SAVE_NOT_UPDATED;
//    }
//  }
//
//  theApp.wasPaused = false;
//
////  Old autoframeskip crap... might be useful later. autoframeskip Ifdef above might be useless as well now
////  theApp.autoFrameSkipLastTime = time;
//
//#ifdef LOG_PERFORMANCE
//  if( systemSpeedCounter >= PERFORMANCE_INTERVAL ) {
//	  // log performance every PERFORMANCE_INTERVAL frames
//	  float a = 0.0f;
//	  for( unsigned short i = 0 ; i < PERFORMANCE_INTERVAL ; i++ ) {
//		  a += (float)systemSpeedTable[i];
//	  }
//	  a /= (float)PERFORMANCE_INTERVAL;
//	  log( _T("Speed: %f\n"), a );
//	  systemSpeedCounter = 0;
//  }
//#endif
}