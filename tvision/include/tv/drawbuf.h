/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *

Modified by Robert H�hne to be used for RHIDE.

 *
 *
 */

#if defined( Uses_TDrawBuffer ) && !defined( __TDrawBuffer )
#define __TDrawBuffer

class TDrawBufferBase
{
public:
 enum Types { codePage, unicode16 };
 int getType();
 virtual void *getBuffer() = 0;

protected:
 int type;
};

inline int TDrawBufferBase::getType()
{
 return type;
}

// Simple TDrawBuffer compatible with original class that handles:
// 8 bit character, 8 bits attribute, ...
class TDrawBuffer : public TDrawBufferBase
{
 friend class TView;

public:
 TDrawBuffer() { type=codePage; }

  void moveChar( unsigned indent, char c, unsigned attr, unsigned count );
  void moveStr( unsigned indent, const char *str, unsigned attrs );
  void moveCStr( unsigned indent, const char *str, unsigned attrs );
  void moveBuf( unsigned indent, const void *source,
                unsigned attr, unsigned count );

  void putAttribute( unsigned indent, unsigned attr );
  void putChar( unsigned indent, unsigned c );

  virtual void *getBuffer();

protected:
  uint16 data[maxViewWidth];
};

#define loByte(w)    (((uchar *)&w)[0])
#define hiByte(w)    (((uchar *)&w)[1])

inline void TDrawBuffer::putAttribute( unsigned indent, unsigned attr )
{
    hiByte(data[indent]) = (uchar)attr;
}

inline void TDrawBuffer::putChar( unsigned indent, unsigned c )
{
    loByte(data[indent]) = (uchar)c;
}

// This is a more advanced class to handle 16 bits Unicode encodings plus
// 16 bits attributes.
class TDrawBufferU16 : public TDrawBufferBase
{
 friend class TView;

public:
 TDrawBufferU16() { type=unicode16; }

 void moveChar( unsigned indent, unsigned c, unsigned attr, unsigned count );
 void moveStr( unsigned indent, const uint16 *str, unsigned attrs );
 void moveCStr( unsigned indent, const uint16 *str, uint32 attrs );
 void moveBuf( unsigned indent, const void *source,
               unsigned attr, unsigned count );

 void putAttribute( unsigned indent, unsigned attr );
 void putChar( unsigned indent, unsigned c );

 virtual void *getBuffer();

protected:

 uint16 data[maxViewWidth*2];
};


inline void TDrawBufferU16::putAttribute( unsigned indent, unsigned attr )
{
    data[indent*2+1] = (uint16)attr;
}

inline void TDrawBufferU16::putChar( unsigned indent, unsigned c )
{
    data[indent*2] = (uint16)c;
}

#endif  // Uses_TDrawBuffer

