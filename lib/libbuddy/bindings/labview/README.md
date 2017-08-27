# Buddy Labview Bindings

The LabVIEW binding currently only supports Windows targets.  The bindings to
the C usbhid_buddy library are exposed through a DLL.

## Building

Open the Visual Studio Project in the `buddy/` directory and build the DLL.  You can
build for either x86 (32-bit) or x64 (64-bit) windows.  Take note of your LabVIEW
version as the bit-type must match the LabVIEW version type.

## Using It

The LabVIEW project library file (buddy.lvlib) is contained within the `utility VI/` directory.
It includes two pre-compiled DLLs for x86 (buddy_x86.dll) and x64 (buddy_x64.dll) targets.  By
default, the buddy.lvlib VIs are configured for x64 so you must edit the DLL path in each sub VI
for x86 support.  