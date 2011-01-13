import Image
import sys

def openRGBA(filename):
    image = Image.open(filename)
    if (image.mode != 'RGBA'):
        image = image.convert('RGBA')
    else:
        image = image.copy
    return image

if (len(sys.argv) <= 3):
    print '%s <image> <alpha> <destination>' % (sys.argv[1])
    sys.exit(0)

im1 = openRGBA(sys.argv[1]);
im2 = openRGBA(sys.argv[2]);

alpha = im2.split()[1]
im1.putalpha(alpha)
im1.save(sys.argv[3])
