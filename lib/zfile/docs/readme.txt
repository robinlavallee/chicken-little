                                        ] ZFile r1 [
                                    Released June 4, 2000
										
                               Copyright (C) 2000 Nathan Youngman                                      

] Introduction [

ZFile is a resource system that works with .ZIP files by using zlib for decompression. It is
intended for use by programmers who wish to distribute their application without the clutter of
many individual files.

To create an archive, you just use WinZIP or similar to make a .ZIP file. You can rename the file
if you like (I generally use .DAT). Then using ZFile, you can load files out of the archive on
the fly.

One highly useful feature is support for a pass-through access, so while in debug mode files can
be loaded directly out of a folder rather than the archive. A simple matter of "opening" a folder
instead of opening an archive. All files opened/read will then use that folder instead.

It is also possible to open an archive attached to the end of your executable. The intention is
that you could distribute a game in a single EXE, already compressed, no installation required.
The EXE itself can be compressed with a tool such as ASPack <http://www.aspack.com>, UPX
<http://wildsau.idv.uni-linz.ac.at/mfx/upx.html>, or PKLITE32 <http://www.pkware.com/shareware/>.

Note: Multi-volume ZIP files (span), ZIP encryption (passwords), and old compression used by old
PKZip 1.x are not supported. ZFile is intended for use on Win32 and has not been tested with
other systems.


] Using ZFile [

I'm not going to go into the ZFile API because you have all the source code. This is just a brief
overview to get you started.

ZFile.dsw is a workspace for Visual C++ 5.0. There are two projects: a test application to
demonstrate how to use ZFile, and the ZFile library itself (zfile.lib).

I have included zlib.lib, zlib.h and zconf.h from zlib 1.1.3. You should be able to replace these
with the equivalent files from a newer version of zlib (when available) without any problems.

When you compile ZFile, it will create zfile.lib. Zfile.lib and zfile.h are all you need for your
own projects (zlib.lib is included within zfile.lib).

The archive system is limited to having one file open at a time. My current interface also limits
you to having only one .ZIP file open at a time. This could be changed by using C++ classes.

To attach a ZIP file to the end of an EXE, use the following DOS command:
copy /b original.exe+data.zip new.exe
You can put this command in the post-build step of Visual C for release builds.

Note: If your .ZIP file contains folders, you must specify the folder when opening a file, such
as: OpenFile("images/myimage.jpg");

Take a look at test.cpp and zfile.h for more information on ZFile usage.


] Contact [

ZFile is a library by Nathan Youngman, which is now an integral part of the game engine I am
developing. The included source code is a separate module from before it was integrated with my
error handling system, etc. and represents a weekend of coding.

If you use ZFile in a product, credit in the documentation and/or credits screen would be
appreciated. If you have any bug fixes or improvements to make, please e-mail me with the
details. I would also be interested to know where ZFile is being used.

E-mail: contact@nathany.com
Web site: http://nathany.com


] Acknowledgments [

Reading ZIP files is based upon Gilles Vollant's MiniUnz 0.15 application (included with the zlib
distribution). <http://www.winimage.com/zLibDll/unzip.html>

ZFile makes use of the zlib library for decompression. Zlib was written by Jean-loup Gailly and
Mark Adler. <ftp://ftp.freesoftware.com/pub/infozip/zlib/zlib.html>

The deflate format used by zlib was defined by Phil Katz. The deflate
and zlib specifications were written by L. Peter Deutsch.  

  
] License [

Copyright (C) 2000 Nathan Youngman

This software is provided 'as-is', without any express or implied warranty.  In no event will the
authors be held liable for any damages arising from the use of this software.

Permission is granted to anyone to use this software for any purpose, including commercial
applications, and to alter it and redistribute it freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote 
   the original software. If you use this software in a product, an acknowledgment in the 
   product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as
   being the original software.
3. This notice may not be removed or altered from any source distribution.

