# NOTES

## Build

Download and install Microsoft Visual Studio Community 2022 (64-bit).

Clone https://github.com/OpenEtherCATsociety/SOEM to C:\GitHub\markosankovic\SOEM

Follow the README file in the cloned SOEM repository.

Start Developer Command Prompt for VS 2022.

```
cd C:\GitHub\markosankovic\SOEM
mkdir build
cd build
cmake .. -G "NMake Makefiles"
nmake
```

Follow https://openethercatsociety.github.io/doc/soem/tutorial_8txt.html

## SIMPLE_NG

Get available adapters by running simple_ng.exe:

```
C:\GitHub\markosankovic\SOEM\build\test\simple_ng>simple_ng.exe
Usage: simple_ng IFNAME1
IFNAME1 is the NIC interface name, e.g. 'eth0'

Available adapters:
    - \Device\NPF_{FD0B7951-2776-4401-8AF7-E41F8499534B}  (Microsoft)
    - \Device\NPF_{3DA59371-5EAA-4F68-93E9-19B52CFDE7C6}  (Microsoft)
    - \Device\NPF_{5B5320DE-B749-438A-9AC7-BA01314AAFA2}  (Microsoft)
    - \Device\NPF_{54ECE901-1E8B-4F53-9633-34D85B5935BB}  ()
    - \Device\NPF_{F62779CA-7C30-432D-93E3-4F5F0E2B17DD}  (Microsoft)
```

Network adapter for device DELL DA200:

    \Device\NPF_{54ECE901-1E8B-4F53-9633-34D85B5935BB}

Run:

    C:\GitHub\markosankovic\SOEM\build\test\simple_ng\simple_ng.exe "\Device\NPF_{54ECE901-1E8B-4F53-9633-34D85B5935BB}"

## SOMANET

Gets built with all other tests.

Run:

    C:\GitHub\markosankovic\SOEM\build\test\somanet\somanet.exe

APIs:

- http://localhost:8000/api/getAdapters

## Links

- https://ms-iot.github.io/ROSOnWindows/tutorials/ethercat/soem.html
- http://mongoose.ws/
