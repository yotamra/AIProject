#include <windows.h>
#include <vfw.h>
#include <avi.h>

#pragma comment(lib, "vfw32.lib")

#define ASSERT(x)

using namespace CPP;
using std::string;

BOOL CreateFromPackedDIBPointer(LPBYTE pDIB, int iFrame, AVICallback* cb)
{
  ASSERT(pDIB!=NULL);

  //Creates a full-color (no palette) DIB from a pointer to a

  //full-color memory DIB


  //get the BitmapInfoHeader

  BITMAPINFOHEADER bih;
  RtlMoveMemory(&bih.biSize, pDIB, sizeof(BITMAPINFOHEADER));

  //now get the bitmap bits

  if (bih.biSizeImage < 1)
  {
    return FALSE;
  }

  BYTE* Bits=pDIB+sizeof(BITMAPINFOHEADER);

  //RtlMoveMemory(Bits, pDIB + sizeof(BITMAPINFOHEADER), bih.biSizeImage);

  //and BitmapInfo variable-length UDT

  BYTE memBitmapInfo[40];
  RtlMoveMemory(memBitmapInfo, &bih, sizeof(bih));

  BITMAPFILEHEADER bfh;
  bfh.bfType=19778;    //BM header

  bfh.bfSize=55 + bih.biSizeImage;
  bfh.bfReserved1=0;
  bfh.bfReserved2=0;
  bfh.bfOffBits=sizeof(BITMAPINFOHEADER) + sizeof(BITMAPFILEHEADER); //54

  {
    int y,w=bih.biWidth,h=bih.biHeight;
    Image frame(new CPPImage(w,h,bih.biBitCount/8));
    unsigned packed_pitch=w*bih.biBitCount/8;
    unsigned pitch=packed_pitch;
    while ((pitch&3)!=0) ++pitch;
    byte* src=Bits;
    for(y=h-1;y>=0;--y)
    {
      byte* row=frame->get_row(y);
      std::copy(src,src+packed_pitch,row);
      src+=pitch;
    }
    cb->process_frame(frame);
  }
  /*
  CString FileName;
  FileName.Format("Frame-%05d.bmp", iFrame);
  FILE* fp=fopen(FileName, "wb");
  if (fp!=NULL)
  {
    fwrite(&bfh, sizeof(bfh), 1, fp);
    fwrite(&memBitmapInfo, sizeof(memBitmapInfo), 1, fp);
    fwrite(Bits, bih.biSizeImage, 1, fp);
    fclose(fp);
  }
  else
  {
    TRACE0(_T("Error writing the bitmap file"));
    return FALSE;
  }
  */

  return TRUE;
}


bool extract_avi_frames(const char* szFileName, AVICallback* cb)
{
  AVIFileInit();

  PAVIFILE avi;
  int res=AVIFileOpen(&avi, szFileName, OF_READ, NULL);

  if (res!=AVIERR_OK)
  {
    //an error occures

    if (avi!=NULL)
      AVIFileRelease(avi);

    return false;
  }

  AVIFILEINFO avi_info;
  AVIFileInfo(avi, &avi_info, sizeof(AVIFILEINFO));
  cb->set_frame_rate(avi_info.dwRate / avi_info.dwScale);

  /*
  CString szFileInfo;
  szFileInfo.Format("Dimention: %dx%d\n"
    "Length: %d frames\n"
    "Max bytes per second: %d\n"
    "Samples per second: %d\n"
    "Streams: %d\n"
    "File Type: %d", avi_info.dwWidth,
    avi_info.dwHeight,
    avi_info.dwLength,
    avi_info.dwMaxBytesPerSec,
    (DWORD) (avi_info.dwRate / avi_info.dwScale),
    avi_info.dwStreams,
    avi_info.szFileType);

  AfxMessageBox(szFileInfo, MB_ICONINFORMATION | MB_OK);

  */

  PAVISTREAM pStream;
  res=AVIFileGetStream(avi, &pStream, streamtypeVIDEO /*video stream*/, 
    0 /*first stream*/);

  if (res!=AVIERR_OK)
  {
    if (pStream!=NULL)
      AVIStreamRelease(pStream);

    AVIFileExit();
    return false;
  }

  //do some task with the stream

  int iNumFrames;
  int iFirstFrame;

  iFirstFrame=AVIStreamStart(pStream);
  if (iFirstFrame==-1)
  {
    //Error getteing the frame inside the stream


    if (pStream!=NULL)
      AVIStreamRelease(pStream);

    AVIFileExit();
    return false;
  }

  iNumFrames=AVIStreamLength(pStream);
  if (iNumFrames==-1)
  {
    //Error getteing the number of frames inside the stream


    if (pStream!=NULL)
      AVIStreamRelease(pStream);

    AVIFileExit();
    return false;
  }

  //getting bitmap from frame

  BITMAPINFOHEADER bih;
  ZeroMemory(&bih, sizeof(BITMAPINFOHEADER));

  bih.biBitCount=24;    //24 bit per pixel

  bih.biClrImportant=0;
  bih.biClrUsed = 0;
  bih.biCompression = BI_RGB;
  bih.biPlanes = 1;
  bih.biSize = 40;
  bih.biXPelsPerMeter = 0;
  bih.biYPelsPerMeter = 0;
  //calculate total size of RGBQUAD scanlines (DWORD aligned)

  bih.biSizeImage = (((bih.biWidth * 3) + 3) & 0xFFFC) * bih.biHeight ;

  PGETFRAME pFrame;
  pFrame=AVIStreamGetFrameOpen(pStream, 
    NULL/*(BITMAPINFOHEADER*) AVIGETFRAMEF_BESTDISPLAYFMT*/ /*&bih*/);

  //Get the first frame

  int index=0;
  for (int i=iFirstFrame; i<iNumFrames; i++)
  {
    index= i-iFirstFrame;

    BYTE* pDIB = (BYTE*) AVIStreamGetFrame(pFrame, index);

    CreateFromPackedDIBPointer(pDIB, index, cb);
  }

  AVIStreamGetFrameClose(pFrame);

  //close the stream after finishing the task

  if (pStream!=NULL)
    AVIStreamRelease(pStream);

  AVIFileExit();

  return true;
}







class AVIWriterImpl
{
  PAVIFILE              m_AviFile;
  AVISTREAMINFO         m_StrHdr;
  PAVISTREAM            m_Stream;
  PAVISTREAM            m_CompressedStream;
  float                 m_StartTime;
  float                 m_TimeScale;
  long                  m_Status;
  int                   m_FrameSize;
  int                   m_FrameRate;
  char*                 m_Buffer;
  int                   m_BufferSize;
  long                  m_Current;
  string                m_OutputPath;

  void initialize_avi(int w, int h, int bpp)
  {
    AVIFileInit();
    if (!SUCCEEDED(AVIFileOpen(&m_AviFile,m_OutputPath.c_str(),OF_CREATE|OF_WRITE,NULL)))
      throw "Failed to open AVI file.";
    ZeroMemory(&m_StrHdr, sizeof(m_StrHdr));
    m_StrHdr.fccType                = streamtypeVIDEO;// stream type
    m_StrHdr.fccHandler             = 0;
    m_StrHdr.dwScale                = 1;
    m_StrHdr.dwRate                 = m_FrameRate;
    m_StrHdr.dwSuggestedBufferSize  = 0; // alpbi[0]->biSizeImage;
    SetRect(&m_StrHdr.rcFrame, 0, 0, w, h);
    if (!SUCCEEDED(AVIFileCreateStream(m_AviFile,&m_Stream,&m_StrHdr)))
      throw "Failed to create AVI Stream";

    {
      AVICOMPRESSOPTIONS opts;
      AVICOMPRESSOPTIONS FAR * aopts[1] = {&opts};
      ZeroMemory(&opts,sizeof(opts));
      HWND hWnd=GetDesktopWindow();
      int rc = AVISaveOptions(hWnd, 0, 1, &m_Stream, (LPAVICOMPRESSOPTIONS FAR *) &aopts);
      m_Status=!rc;
      if (m_Status != 0) return;
      m_Status = AVIMakeCompressedStream(&m_CompressedStream, m_Stream, &opts, NULL);
      if (m_Status != 0) throw "Failed to create compressed stream";
    }

    int pitch=w*bpp/8;
    while ((pitch&3)!=0) ++pitch;
    BITMAPINFOHEADER bih;
    ZeroMemory(&bih,sizeof(BITMAPINFOHEADER));
    bih.biSize=sizeof(BITMAPINFOHEADER);
    bih.biWidth=w;
    bih.biHeight=h;
    bih.biPlanes=1;
    bih.biBitCount=bpp;
    bih.biCompression=0;
    bih.biSizeImage=pitch*h;
    
    if (!SUCCEEDED(AVIStreamSetFormat(m_CompressedStream,0,&bih,sizeof(BITMAPINFOHEADER))))
      throw "Failed to set AVI format";
  }

  void close_avi_file()
  {
    if (m_CompressedStream!=NULL && m_CompressedStream!=m_Stream)
      AVIStreamClose(m_CompressedStream);
    if (m_Stream!=NULL) AVIStreamClose(m_Stream);
    if (m_AviFile!=NULL) AVIFileClose(m_AviFile);
    AVIFileExit();
  }

public:
  AVIWriterImpl(const string& output_name, int w, int h, int bpp, int frame_rate)
    : m_OutputPath(output_name),
      m_FrameRate(frame_rate),
      m_FrameSize(w*h*bpp/8),
      m_Current(0)
  {
    initialize_avi(w,h,bpp);
  }

  ~AVIWriterImpl() 
  { 
    close_avi_file(); 
  }

  void write_frame(Image frame)
  {
    frame=flip_vertical(frame);
    const byte* buffer=frame->get_row(0);
    long rc=AVIStreamWrite(m_CompressedStream,m_Current,1,(LPVOID)buffer,m_FrameSize,AVIIF_KEYFRAME,NULL,NULL);
    ++m_Current;
  }
};


AVIWriter::AVIWriter(const char* filename, int w, int h, int bpp, int frame_rate)
: m_Impl(new AVIWriterImpl(filename,w,h,bpp,frame_rate))
{
}

AVIWriter::~AVIWriter()
{
  delete m_Impl;
  m_Impl=0;
}

void AVIWriter::write_frame(Image frame)
{
  m_Impl->write_frame(frame);
}
