# NeopixelClock
Clock with Neopixel Ring or tree 6-pixels NeoPixel Strips and Wemos D1 Board. 

Parts:

- 60 pixels Neopixel Ring
	- and/or tree 6-pixels NeoPixel Strips
- Wemos D1 Board

The Clock repentates the Time of ntp.nist.gov as coloured Pixels (see project-photo.jpg):
- Hour red Pixel
- Minutes green Pixel
- Seconds blue Pixel

The Hour-classifications are representated by white Pixels

Binary Clock:
Tree Strips representing the Clock:

  32  16  8   4   2   1    = Binary
+-----------------------+
| N | o | o | o | o | o | = hour  N = Representing 12 / 24 o'Clock by indicating Yellow for Day, red for Night
+-----------------------+
| x | x | x | x | x | x | = minutes
+-----------------------+
| # | # | # | # | # | # | = seconds
+-----------------------+


Happy building


