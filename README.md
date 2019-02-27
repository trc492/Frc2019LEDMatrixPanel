# Arduino text/image functions for the Adafruit 64x32 LED Matrix Panel
### Image support
This system consists of two different files: one that runs on the Arduino MEGA and a Python file that you run locally.
The Arduino project is designed to take in 8 chunks of an image (24-bit RGB) and display them on the board in series. It uses an 57600-baud serial connection to communicate with the Python script, which sends it another chunk of the image when it recieves a signal.
To run this, just load the project onto the Arduino and run the Python script. You can change the images that are loaded by going into the file and modifying the `images_to_load` array at the top, but just make sure they're 64x32. The computer _does_ need to stay plugged into the Arduino because of the ongoing serial connection.
Brightness and contrast adjustors are built in because sometimes images need to be modified for them to look good on the display. Those can also be found at the top of the file in constants named `CONTRAST_ADJUSTMENT` and `BRIGHTNESS_ADJUSTMENT`. A contrast adjustor of 2 and a brightness adjustor of 0.5 seems to work quite well, but feel free to tinker with these.
### Text support
Does not currently work. Working on a fix...
