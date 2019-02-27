from PIL import Image, ImageEnhance
import serial
import itertools
import io
import time

# A list of image files to load
images_to_load = [
    "megaman-is.bmp",
    "megaman-boeing.bmp",
    "megaman-microsoft.bmp"
]

# Contrast and brightness used to adjust the image before sending
CONTRAST_ADJUSTMENT = 2
BRIGHTNESS_ADJUSTMENT = 0.5

class SplitImage:
    def __init__(self, path, number_of_slices, number_of_lines):
        self.arrays = []
        self.array_index = 0
        
        im = Image.open(path)

        # Make sure the image is in RGB colorspace (24-bit)
        im = im.convert("RGB")

        # Adjust the contrast of the image
        contrast = ImageEnhance.Contrast(im)
        im = contrast.enhance(CONTRAST_ADJUSTMENT)

        # Adjust the contrast of the image
        brightness = ImageEnhance.Brightness(im)
        im = brightness.enhance(BRIGHTNESS_ADJUSTMENT)

        # Basically splitting the image data into 8
        byte_array = b''.join(list(map(bytes, im.getdata())))

        i = iter(byte_array)
        for o in range(number_of_slices):
            self.arrays.append(bytes(itertools.islice(i, int(len(byte_array)/number_of_slices))))

    def current_slice(self):
        to_send = self.arrays[self.array_index]
        if self.array_index == len(self.arrays) - 1:
            self.array_index = 0
        else:
            self.array_index += 1
        return to_send

    def print_slices(self):
        for s in self.arrays:
            print(len(s))
            print(s)

class Frames:
    def __init__(self, sections, lines):
        self.images = []
        self.sections = sections
        self.frame_index = 0
        self.lines = lines

    def add(self, filename):
        self.images.append(SplitImage(filename, self.sections, self.lines))

    def get_slice(self):
        if self.images[self.frame_index].array_index == self.sections - 1:
            s = self.images[self.frame_index].current_slice()
            if self.frame_index < len(self.images) - 1:
                self.frame_index += 1
            else:
                self.frame_index = 0
        else:
            s = self.images[self.frame_index].current_slice()
        return s

    def get_line(self):
        if self.images[self.frame_index].line_index == self.sections - 1:
            l = self.images[self.frame_index].current_line()
            if self.frame_index < len(self.images) - 1:
                self.frame_index += 1
            else:
                self.frame_index = 0
        else:
            l = self.images[self.frame_index].current_line()
        return l

number_of_slices = 8

frames = Frames(number_of_slices, number_of_lines)

ser = serial.Serial('COM3', 57600)


for i in images_to_load:
    frames.add(i)


while(1):
    string = ""
    if ser.in_waiting > 0:
        while ser.in_waiting > 0:
            string += ser.read().decode("utf-8")
            time.sleep(0.005)
        print('From Arduino: %s'%string)
        if 'send request' in string:
            s = frames.get_slice()
            print('SENDING %d BYTES' % len(s))
            ser.write(s)

