/* X11 keyboard routines.
   Copyright (c) 2001 by Salvador E. Tropea (SET)
   Covered by the GPL license. */
#include <tv/configtv.h>

#if defined(TVOS_UNIX) && defined(HAVE_X11)
#define Uses_TDisplay
#define Uses_TScreen
#define Uses_TGKey
#define Uses_FullSingleKeySymbols
#define Uses_TEvent
#include <tv.h>

// X11 defines their own values
#undef True
#undef False
#include <X11/Xlib.h>
#include <X11/keysym.h>

#include <tv/x11/screen.h>
#include <tv/x11/key.h>

#include <stdio.h>
#include <X11/XKBlib.h>

/*****************************************************************************

  TGKeyX11 keyboard stuff.

*****************************************************************************/

int      TGKeyX11::lenKb;
int      TGKeyX11::kbWaiting=0;
char     TGKeyX11::bufferKb[MaxKbLen+1];
KeySym   TGKeyX11::Key;
unsigned TGKeyX11::kbFlags=0;
uchar    TGKeyX11::KeyCodeByKeySym[256];
unsigned TGKeyX11::Symbol;
unsigned TGKeyX11::Flags;
uchar    TGKeyX11::Scan;

int TGKeyX11::getKeyEvent(int block)
{
 Status status; // Currently ignored, but Solaris doesn't support NULL as argument
 XEvent event;

 /* If we already have a key use it */
 if (kbWaiting)
    return 1;

 while (1)
   {
    /* Get the next keyboard event */
    if (XCheckMaskEvent(TScreenX11::disp,aKeyEvent,&event)!=True)
      {
       TScreenX11::ProcessGenericEvents();
       if (block)
          continue;
       return 0;
      }
    /* Now check if it is a key that will be combined */
    if (XFilterEvent(&event,0)==True)
       continue;
    if (event.type==KeyPress)
      {
       printf("Key event 0x%04X\n",event.xkey.state);
       lenKb=XmbLookupString(TScreenX11::xic,&event.xkey,bufferKb,MaxKbLen,&Key,&status);
       bufferKb[lenKb]=0;
       /*printf("Key event %d %s 0x%04X\n",lenKb,bufferKb,Key);*/
       /* FIXME: how can I know the state before entering the application? */
       #define ToggleBit(a) if (kbFlags & a) kbFlags&=~a; else kbFlags|=a
       /* Look if that's a modifier */
       switch (Key)
         {
          case XK_Shift_R:
               kbFlags|=kbRightShiftDown;
               break;
          case XK_Shift_L:
               kbFlags|=kbLeftShiftDown;
               break;
          case XK_Control_R:
               kbFlags|=kbRightCtrlDown | kbCtrlDown;
               break;
          case XK_Control_L:
               kbFlags|=kbLeftCtrlDown | kbCtrlDown;
               break;
          case XK_Alt_R:
               kbFlags|=kbRightAltDown | kbAltDown;
               break;
          case XK_Alt_L:
               kbFlags|=kbLeftAltDown | kbAltDown;
               break;
          case XK_Scroll_Lock:
               kbFlags|=kbScrollLockDown;
               ToggleBit(kbScrollLockToggle);
               break;
          case XK_Num_Lock:
               kbFlags|=kbNumLockDown;
               ToggleBit(kbNumLockToggle);
               break;
          case XK_Caps_Lock:
               kbFlags|=kbCapsLockDown;
               ToggleBit(kbCapsLockToggle);
               break;
          case XK_Insert:
               ToggleBit(kbInsertToggle);
               kbWaiting=1;
               return 1; /* Report the key */
          case XK_Sys_Req:
               kbFlags|=kbSysReqPress;
               kbWaiting=1;
               return 1; /* Report the key */
          default:
               /* Extract the modifiers info: */
               if (event.xkey.state & ShiftMask)
                 {
                  if (!(kbFlags & (kbRightShiftDown | kbLeftShiftDown)))                    
                     kbFlags|=kbLeftShiftDown;                     
                 }
               else
                 {
                  if (kbFlags & (kbRightShiftDown | kbLeftShiftDown))
                     kbFlags&=~(kbRightShiftDown | kbLeftShiftDown);
                 }
               if (event.xkey.state & LockMask)
                  kbFlags|=kbCapsLockDown;
               else
                  kbFlags&=~kbCapsLockDown;
               if (event.xkey.state & ControlMask)
                 {
                  if (!(kbFlags & kbCtrlDown))
                     kbFlags|=kbCtrlDown | kbLeftCtrlDown;
                 }
               else
                 {
                  if (kbFlags & kbCtrlDown)
                     kbFlags&=~(kbLeftCtrlDown | kbRightCtrlDown | kbCtrlDown);
                 }
               if (event.xkey.state & Mod1Mask) // Alt for XFree86
                 {
                  if (!(kbFlags & kbAltDown))
                     kbFlags|=kbAltDown | kbLeftAltDown;
                 }
               else
                 {
                  if (kbFlags & kbAltDown)
                    {
                     printf("Quitando ALT que no es v�lido\n");
                     kbFlags&=~(kbLeftAltDown | kbRightAltDown | kbAltDown);
                    }
                 }
               if (event.xkey.state & Mod2Mask) // NumLock for XFree86
                  kbFlags|=kbNumLockDown;
               else
                  kbFlags&=~kbNumLockDown;
               if (event.xkey.state & Mod5Mask) // ScrollLock for XFree86
                  kbFlags|=kbScrollLockDown;
               else
                  kbFlags&=~kbScrollLockDown;
               kbWaiting=1;
               return 1;
         }
       #undef ToggleBit
      }
    else
      {
       KeySym k=XLookupKeysym(&event.xkey,0);
       switch (k)
         {
          case XK_Shift_R:
               kbFlags&=~kbRightShiftDown;
               break;
          case XK_Shift_L:
               kbFlags&=~kbLeftShiftDown;
               break;
          case XK_Control_R:
               kbFlags&=~kbRightCtrlDown;
               if ((kbFlags & kbLeftCtrlDown)==0)
                  kbFlags&=~kbCtrlDown;
               break;
          case XK_Control_L:
               kbFlags&=~kbLeftCtrlDown;
               if ((kbFlags & kbRightCtrlDown)==0)
                  kbFlags&=~kbCtrlDown;
               break;
          case XK_Alt_R:
               kbFlags&=~kbRightAltDown;
               if ((kbFlags & kbLeftAltDown)==0)
                  kbFlags&=~kbAltDown;
               break;
          case XK_Alt_L:
               kbFlags&=~kbLeftAltDown;
               if ((kbFlags & kbRightAltDown)==0)
                  kbFlags&=~kbAltDown;
               break;
          case XK_Scroll_Lock:
               kbFlags&=~kbScrollLockDown;
               break;
          case XK_Num_Lock:
               kbFlags&=~kbNumLockDown;
               break;
          case XK_Caps_Lock:
               kbFlags&=~kbCapsLockDown;
               break;
         }
      }
   }
 return 0;
}

int TGKeyX11::KbHit()
{
 return getKeyEvent(0);
}

keyEquiv TGKeyX11::XEquiv[]=
{
 /* Cursor control & motion */
 {XK_Home,kbHome}, {XK_Left,kbLeft}, {XK_Up,kbUp}, {XK_Right,kbRight},
 {XK_Down,kbDown}, {XK_Page_Up,kbPgUp}, {XK_Page_Down, kbPgDn},
 {XK_End,kbEnd}, {XK_Begin,kbHome},
 /* Misc Functions */
 {XK_Insert, kbInsert}, {XK_Delete, kbDelete},
 /* TTY Functions */
 {XK_Return, kbEnter}, {XK_BackSpace, kbBackSpace}, {XK_Tab, kbTab},
 {XK_Escape, kbEsc},
 /* Auxilliary Functions */
 {XK_F1, kbF1}, {XK_F2, kbF2}, {XK_F3, kbF3}, {XK_F4, kbF4}, {XK_F5, kbF5},
 {XK_F6, kbF6}, {XK_F7, kbF7}, {XK_F8, kbF8}, {XK_F9, kbF9}, {XK_F10, kbF10},
 {XK_F11, kbF11}, {XK_F12, kbF12},
 /* Keypad Functions */
 {XK_KP_0,kb0}, {XK_KP_1,kb1}, {XK_KP_2,kb2}, {XK_KP_3,kb3}, {XK_KP_4,kb4},
 {XK_KP_5,kb5}, {XK_KP_6,kb6}, {XK_KP_7,kb7}, {XK_KP_8,kb8}, {XK_KP_9,kb9},
 {XK_KP_Enter, kbEnter}, {XK_KP_Home,kbHome}, {XK_KP_Left,kbLeft},
 {XK_KP_Up, kbUp}, {XK_KP_Right,kbRight}, {XK_KP_Down, kbDown},
 {XK_KP_Page_Up, kbPgUp}, {XK_KP_Page_Down,kbPgDn}, {XK_KP_End,kbEnd},
 {XK_KP_Begin, kbHome}, {XK_KP_Insert, kbInsert}, {XK_KP_Delete, kbDelete},
 {XK_KP_Equal, kbEqual}, {XK_KP_Multiply, kbAsterisk}, {XK_KP_Add, kbPlus},
 {XK_KP_Subtract, kbMinus}, {XK_KP_Decimal, kbStop}, {XK_KP_Divide, kbSlash},
 /* End */
 {0,0}
};

uchar TGKeyX11::KeyCodeByASCII[96]=
{
 kbSpace,kbAdmid,kbDobleQuote,kbNumeral,kbDolar,kbPercent,kbAmper,kbQuote,
 kbOpenPar,kbClosePar,kbAsterisk,kbPlus,kbComma,kbMinus,kbStop,kbSlash,
 kb0,kb1,kb2,kb3,kb4,kb5,kb6,kb7,
 kb8,kb9,kbDoubleDot,kbColon,kbLessThan,kbEqual,kbGreaterThan,kbQuestion,
 kbA_Roba,kbA,kbB,kbC,kbD,kbE,kbF,kbG,
 kbH,kbI,kbJ,kbK,kbL,kbM,kbN,kbO,
 kbP,kbQ,kbR,kbS,kbT,kbU,kbV,kbW,
 kbX,kbY,kbZ,kbOpenBrace,kbBackSlash,kbCloseBrace,kbCaret,kbUnderLine,
 kbGrave,kbA,kbB,kbC,kbD,kbE,kbF,kbG,
 kbH,kbI,kbJ,kbK,kbL,kbM,kbN,kbO,
 kbP,kbQ,kbR,kbS,kbT,kbU,kbV,kbW,
 kbX,kbY,kbZ,kbOpenCurly,kbOr,kbCloseCurly,kbTilde,kbBackSpace
};

ushort TGKeyX11::GKey()
{
 unsigned name;

 getKeyEvent(1);
 kbWaiting=0;

 if ((Key & 0xFF00)==0xFF00)
   {/* Special keys by keysym */
    Symbol=(unsigned char)bufferKb[0];
    name=KeyCodeByKeySym[Key & 0xFF];
   }
 else if (lenKb==1)
   {/* A key by their ASCII */
    Symbol=(unsigned char)bufferKb[0];
    if (Symbol>=32 && Symbol<128)
       name=KeyCodeByASCII[Symbol-32];
    else
       name=kbUnkNown;
   }
 else
   {/* A key with something else, not handled yet */
    Symbol=0;
    name=kbUnkNown;
   }
 Scan=Key & 0xFF;

 /* Process the flags, just like if it came from an IBM BIOS */
 Flags=0;
 if (kbFlags & (kbLeftShiftDown | kbRightShiftDown))
    Flags|=kbShiftCode;
 if (kbFlags & kbCtrlDown)
    Flags|=kbCtrlCode;
 switch (AltSet)
   {
    case 0: // Normal thing, left is left, right is right
         if (kbFlags & kbLeftAltDown)
            Flags|=kbAltLCode;
         else
            if (kbFlags & kbAltDown)
               Flags|=kbAltRCode;
         break;
    case 1: // Reverse thing
         if (kbFlags & kbLeftAltDown)
            Flags|=kbAltRCode;
         else
            if (kbFlags & kbAltDown)
               Flags|=kbAltLCode;
         break;
    default: // Compatibility
         if (kbFlags & (kbLeftAltDown | kbAltDown))
            Flags|=kbAltLCode;
   }

 return name|Flags;
}

unsigned TGKeyX11::GetShiftState()
{
 return kbFlags;
}

void TGKeyX11::FillTEvent(TEvent &e)
{
 ushort Abstract=GKey();
 e.keyDown.charScan.charCode=Flags & kbAltLCode ? 0 : Symbol;
 e.keyDown.charScan.scanCode=Scan;
 e.keyDown.raw_scanCode=Scan;
 e.keyDown.keyCode=Abstract;
 e.keyDown.shiftState=Flags;
 e.what=evKeyDown;
}

void TGKeyX11::Init()
{
 TGKey::kbhit        =KbHit;
 TGKey::gkey         =GKey;
 TGKey::getShiftState=GetShiftState;
 TGKey::fillTEvent   =FillTEvent;
 /* Initialize keyboard tables */
 for (int i=0; XEquiv[i].symbol; i++)
     KeyCodeByKeySym[XEquiv[i].symbol & 0xFF]=XEquiv[i].key;
}
#endif // defined(TVOS_UNIX) && defined(HAVE_X11)

