#!/usr/bin/env python
import time
import sys
import math
import random

from rgbmatrix import RGBMatrix, RGBMatrixOptions
from PIL import Image, ImageEnhance

# The number of rows and columns on the matrix
ROWS = 64
COLS = 64

# Brightness and contrast multipliers
BRIGHTNESS = 0.5
CONTRAST = 2

# The time value can be left out, and if so it will default to 10 seconds.
images = [
    {"filename": "trc-492-space.png", "time": 5},
    {"filename": "thanks.png", "time": 5}
]

double_buffer = None
matrix = None

# The below code generates the series of pixels that must be
# filled in sequentially to generate circles of incrementing
# size. It is much more efficient to only do this calculation once,
# as generation will not cause lag later down the line.
circles = []
for radius in range(math.ceil(math.sqrt((ROWS/2) ** 2 + (COLS/2) ** 2))): # use the pythagorean theorem to get the distance from center to corner
    circles.append([])
    for r in range(ROWS):
        for c in range(COLS):
            distance_from_center = math.sqrt((r - (ROWS-1)/2) ** 2 + (c - (COLS-1)/2) ** 2)
            if distance_from_center <= radius and distance_from_center > radius - 1: # if pixel in circle but not in previous circle
                circles[-1].append((c, r))

def list_sum(l):
    """Combines a list's sublists into one list"""
    final = []
    for e in l:
        final += e
    return final

class Transition:
    """A class containing various image transition methods"""
    
    UP = 0
    RIGHT = 1
    LEFT = 2
    DOWN = 3
    SQUARE = 4
    CIRCLE = 5
    def wipe(image, speed, direction=UP, blinds=False):
        """Wipes the screen in the specified direction. Blinds makes the effect apply both ways (not applicable to circular and square wipes)."""
        global matrix
        if direction in [Transition.UP, Transition.DOWN]:
            for r in range(math.ceil(ROWS/2) if blinds else ROWS):
                for c in range(COLS):
                    pixel_color = image.getpixel((c, ROWS-r-1 if direction == Transition.UP else r))[:3]
                    matrix.SetPixel(c,
                                    (ROWS-r-1 if direction == Transition.UP else r),
                                    *pixel_color)
                    if blinds:
                        pixel_color = image.getpixel((c, ROWS-r-1 if direction == Transition.DOWN else r))[:3]
                        matrix.SetPixel(c,
                                        (ROWS-r-1 if direction == Transition.DOWN else r),
                                        *pixel_color)
                time.sleep(1/(speed / 10))
        elif direction in [Transition.RIGHT, Transition.LEFT]:
            for c in range(math.ceil(COLS/2) if blinds else COLS):
                for r in range(ROWS):
                    pixel_color = image.getpixel((COLS-c-1 if direction == Transition.LEFT else c, r))[:3]
                    matrix.SetPixel(COLS-c-1 if direction == Transition.LEFT else c,
                                    r,
                                    *pixel_color)
                    if blinds:
                        pixel_color = image.getpixel((COLS-c-1 if direction == Transition.RIGHT else c, r))[:3]
                        matrix.SetPixel(COLS-c-1 if direction == Transition.RIGHT else c,
                                        r,
                                        *pixel_color)
                time.sleep(1/(speed / 10))
        elif direction == Transition.SQUARE:
            for i in range(math.ceil(max(ROWS, COLS)/2)):
                for r in range(i * 2):
                    for c in range(i * 2):
                        if (r not in [0, i * 2 - 1]) and (c not in [0, i * 2 - 1]): # don't worry about the center, its already been set
                            continue
                        coords = (COLS/2-i+c, ROWS/2-i+r)
                        pixel_color = image.getpixel(coords)[:3]
                        matrix.SetPixel(*coords, *pixel_color)
                time.sleep(1/(speed / 10))
        elif direction == Transition.CIRCLE:
            for circle in circles:
                for coords in circle:
                    pixel_color = image.getpixel(coords)[:3]
                    matrix.SetPixel(*coords, *pixel_color)
                time.sleep(1/(speed / 10))

    def noise(image, speed):
        """Changes random pixels on the matrix until the picture change is complete."""
        global matrix
        remaining = list_sum([[[i, o] for i in range(COLS)] for o in range(ROWS)])
        while len(remaining) != 0:
            for i in range(speed):
                if len(remaining) == 0:
	                    break
                coords = random.choice(remaining)
                pixel_color = image.getpixel(tuple(coords))[:3]
                matrix.SetPixel(*coords, *pixel_color)
                remaining.remove(coords)
            time.sleep(1/(speed/10))

    def fade(current_image, next_image, speed):
        """Fades between two images. I'm honestly not sure if this one works properly, but it should."""
        global matrix
        for i in range(255):
            blended = Image.blend(current_image.convert("RGB"), next_image.convert("RGB"), alpha=i/255)
            matrix.SetImage(blended)
            time.sleep(1/(speed/10))

    def random(current_image, next_image):
        """Chooses a random transition to use (currently will never choose fade)."""
        rand = random.randint(1, 8)
        if rand == 1:
            Transition.noise(next_image, 500)
        else:
            Transition.wipe(next_image, 500, random.randint(0, 5), random.randint(0, 1))
                

def fit(image, x, y, method=Image.ANTIALIAS):
    """Scales the image to fit the screen."""
    x_diff = x / image.width
    y_diff = y / image.height
    diff = min(x_diff, y_diff)
    return image.resize((round(image.width * diff), round(image.height * diff)), method)

# Configuration for the matrix
options = RGBMatrixOptions()
options.rows = ROWS
options.cols = COLS
options.chain_length = 1
options.parallel = 1
options.hardware_mapping = 'adafruit-hat'
options.gpio_slowdown = 1
options.show_refresh_rate = False
options.brightness = 100

matrix = RGBMatrix(options = options)

double_buffer = matrix.CreateFrameCanvas()

# Prepare all of the images, add an "image" key to the dictionaries and assign the Image object to it
for image_data in images:
    image = fit(Image.open(image_data['filename']), matrix.width, matrix.height, Image.NEAREST)
    image = ImageEnhance.Brightness(image).enhance(BRIGHTNESS)
    image = ImageEnhance.Contrast(image).enhance(CONTRAST)
    image_data['image'] = image

try:
    print("Press CTRL-C to stop.")
    while(1):
        for i in range(len(images)):
            image_data = images[i]
            print("Displaying image %s for %d seconds..." % (image_data["filename"], image_data["time"]))
            image = image_data['image']
            matrix.SetImage(image.convert('RGB'))
            try:
                time.sleep(image_data['time'])
            except KeyError:
                time.sleep(10)
            next_index = i + 1 if i != len(images) - 1 else 0

            # As stated up above, I'm fairly sure fade works properly but not 100% sure. Be aware of that.
            
            #Transition.wipe(images[next_index]["image"], 500, Transition.CIRCLE, blinds=True)
            #Transition.noise(images[next_index]["image"], 500)
            #Transition.fade(image, images[next_index]["image"], 500)
            Transition.random(image, images[next_index]["image"])

except KeyboardInterrupt:
    sys.exit(0)
