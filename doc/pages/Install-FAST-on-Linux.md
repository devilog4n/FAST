Install FAST for Ubuntu Linux {#install-ubuntu-linux}
===========================

To install FAST on Ubuntu Linux (18.04 or newer), first make sure you have all the necessary [requirements installed](@ref requirements).

Then, download the debian installer (fast_ubuntu18.04_X.X.X.deb) from the [release page](https://github.com/smistad/FAST/releases).
Install the package:
```bash
sudo dpkg -i fast_ubuntu18.04_X.X.X.deb
```
FAST will then be installed to */opt/fast/*

To test if your FAST installation works, you can the following:
```bash
# Download the test data (~2GB), it will be downloaded to /home/<your username>/FAST/data/
cd /opt/fast/bin/
./downloadTestData
# Run an example
./importImageFromFile
```
An ultrasound image should now appear on your screen.

Troubleshoot
------------------

If the installer fails you can download .tar.xz file instead, and just extract it to anywhere on your drive.

Uninstall
------------------

To uninstall FAST:
```bash
sudo apt remove fast
```

To delete the test data and any cached data, delete the folder /home/'your username'/FAST/ as well.