# Windows HEIC Thumbnail Provider

Windows 10 does not support HEIC files by default, which are the native photo image format of recent iPhones.

HEIC files are similar to JPEG files, but with better quality in half the file size.

This small shell extension adds the ability for Windows Explorer to display thumbnails of HEIC files.

![20220606-201945-explorer](https://user-images.githubusercontent.com/323682/172850354-902dbd7d-686f-4749-acc5-23990e65128e.png)

To open or edit HEIC files you'll still need another application such as [Paint.NET](https://www.getpaint.net/) or [Krita](https://krita.org/).

# Installing

Requires Windows 10 (64-bit)

- Install the latest [Microsoft Visual C++ Redistributable](https://aka.ms/vs/17/release/vc_redist.x64.exe) if needed.
- Download the [latest release of HEICThumbnailHandler](https://github.com/brookmiles/windows-heic-thumbnails/releases/latest).
- Extract the files `HEICThumbnailHandler.dll`, `heif.dll`, and `libde265.dll` into a new folder of your choosing.
- Run `regsvr32 HEICThumbnailHandler.dll`

Windows Explorer should now display thumbnails for HEIC files.

# Building

This project was built with Visual Studio 2022.

Requires [libheif](https://github.com/strukturag/libheif) which can be installed with [vcpkg](https://github.com/microsoft/vcpkg).

`vcpkg install libheif:x64-windows`

Optionally use the included vcpkg overlay which removes the dependancy on the x265 encoder, a 5MB dll which is not used.

`vcpkg install libheif:x64-windows --overlay-ports=..\windows-heic-thumbnails\vcpkg-overlay`
