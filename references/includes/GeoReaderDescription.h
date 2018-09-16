// GeoReader.h
// Copyright (c) 2011 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_GeoReaderDescription_h
#define DDImage_GeoReaderDescription_h

#include "DDImage/ReaderDescription.h"
#include "DDImage/Reader.h"

#define USE_DEPRECATED_READ_GEO_CTOR 1

namespace DD
{
  namespace Image
  {

class ReadGeo;
class GeoReader;
class Description;
class GeoReaderFormat;

/*! 
    This structure describes a subclass of GeoReader. The constructor
    builds these into a list that ReadGeo operators search to find a way
    to read a filename given to them.

    Sample code for a plugin that defines a GeoReader:

    \code
   class MyReader : public GeoReader {
   static const Description d;
   ...
   };

   static GeoReader* build(int fd, const unsigned char* b, int n) {
   return new MyReader(fd, b, n);
   }

   static bool test(int fd, const unsigned char *block, int) {
   return (block[0] == MY_MAGIC_BYTE);
   }

   const GeoReader::Description MyReader::d("myf\0", "my file format", build, test);
    \endcode
 */
class DDImage_API GeoDescription : public DD::Image::ReaderDescription
{
private:
  static void add(DD::Image::Description*);
public:
  typedef GeoReader* (*Constructor)(ReadGeo *, int fd, const unsigned char* buf, int bufsize);
  typedef bool (* Tester)(int fd, const unsigned char* buf, int bufsize);

  /*! A list of pointers to all possible Descriptions to be
      used for instantiation of geo plugins. */
  static DD::Image::Description::DescriptionList _collection;

  /*! Make an instance of the reader.

     If test is not null then \a fd is an opened file, it is
     seeked to \a bufsize, and \a buf contains the first
     \a bufsize bytes of the file. You can use this data to
     store stuff into the info_ (however you cannot report
     error messages and filename() does not work yet). */
  Constructor _constructor;

  typedef GeoReaderFormat* (*GeoReaderFormatConstructor)(ReadGeo*);

  GeoReaderFormatConstructor _format_constructor;

  /*! If this pointer is non-null, it indicates a named Unix file is
     used to store the image.  In this case Read will open the file
     and call this function.  If it returns non-zero then the constructor
     will be called.  This allows Read to poll all the Readers to
     see which one can read an unknown file format.

     If this is null then zero is passed as the fd to the constructors.

     \a buf points at the first \a bufsize characters of the file.
     \a bufsize is at least 512.  If necessary you can mess with the
     \a fd, but be sure to seek it back to \a bufsize.
   */
  Tester _test;

  /**
   * whether or not the file descriptor is really needed (defaults to true)
   * 
   * if false then any open FD on the file will be closed before build is called,
   * and -1 passed in
   */
  bool _needFd;

#if USE_DEPRECATED_READ_GEO_CTOR
  /*! Constructor that fills all fields and allows a license check. */
  GeoDescription(const char* n, const char* l, Constructor c, Tester t = 0, License * f = 0, bool needFd = true)
    : ReaderDescription(n, l)
    , _constructor(c)
    , _format_constructor(NULL)
    , _test(t)
    , _needFd(needFd)
  {
    license = f;
    ctor(add);
  }

  /*! Simpler constructor that sets names and label to same value. */
  GeoDescription(const char* n, Constructor c, Tester t = 0, License * f = 0, bool needFd = true)
    : ReaderDescription(n, n)
    , _constructor(c)
    , _format_constructor(NULL)
    , _test(t)
    , _needFd(needFd)
  {
    license = f;
    ctor(add);
  }

#endif // USE_DEPRECATED_READ_GEO_CTOR

  /*! Simpler constructor that sets names and label to same value. */
  GeoDescription(const char* n, Constructor c, GeoReaderFormatConstructor fmtConstructor, Tester t = 0, License * f = 0, bool needFd = true)
    : ReaderDescription(n, n)
    , _constructor(c)
    , _format_constructor(fmtConstructor)
    , _test(t)
    , _needFd(needFd)
  {
    license = f;
    ctor(add);
  }

  static const GeoDescription* find(int i);
  static const GeoDescription* find(const char* name);
};


  } // namespace Image
} // namespace DD

#endif // DDImage_GeoReaderDescription_h
