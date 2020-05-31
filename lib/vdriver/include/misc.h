
/*  procedure macro defines using bios font output:

      print, shadowprint  - uses text location method (screen res 80x30, 80x60)
      Print, ShadowPrint  - uses pixel location method (screen res 640x480)
*/               
#define print(x,y,t,c)         dispstringb(x<<3,y<<4,c,t)
#define Print(x,y,t,c)         dispstringb(x,y,c,t)
#define shadowprint(x,y,t,c,d) dispstringb((x<<3)+2,(y<<4)+2,d,t); dispstringb(x<<3,y<<4,c,t)
#define ShadowPrint(x,y,t,c,d) dispstringb(x+2,y+2,d,t); dispstringb(x,y,c,t)
#define dispchar(x,y,c,d)      dispcharb(x,y,c,d)

