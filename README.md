# Windows HEIC Thumbnail Provider

Windows 10 does not support HEIC files by default, which are the native photo image format of recent iPhones.

HEIC files are similar to JPEG files, but with better quality in half the file size.

This small shell extension adds the ability for Windows Explorer to display thumbnails of HEIC files.

![20220606-201945-explorer](https://user-images.githubusercontent.com/323682/172850354-902dbd7d-686f-4749-acc5-23990e65128e.png)

To open or edit HEIC files you'll still need another application such as [Paint.NET](https://www.getpaint.net/) or [Krita](https://krita.org/).

# Building

This project was built with Visual Studio 2022.

Requires [libheif](https://github.com/strukturag/libheif) which can be installed with [vcpkg](https://github.com/microsoft/vcpkg).

`vcpkg install libheif:x64-windows`

Optionally use the included vcpkg overlay which removes the dependancy on the x265 encoder, a 5MB dll which is not used.

`vcpkg install libheif:x64-windows --overlay-ports=..\windows-heic-thumbnails\vcpkg-overlay`
